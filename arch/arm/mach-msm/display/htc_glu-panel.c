#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <asm/mach-types.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/debug_display.h>
#include "../../../../drivers/video/msm/mdss/mdss_dsi.h"

/* HTC: dsi_power_data overwrite the role of dsi_drv_cm_data
   in mdss_dsi_ctrl_pdata structure */
struct dsi_power_data {
	uint32_t sysrev;         /* system revision info */
	struct regulator *vddio; /* 1.8v */
	struct regulator *vdda;  /* 1.2v */

	struct regulator *vlcmio; /* 1.8v */
	int lcmio;
	int lcmp5v;
	int lcmn5v;
	int lcm_bl_en;
};

static int htc_glu_regulator_init(struct platform_device *pdev)
{
	int ret = 0;
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dsi_power_data *pwrdata = NULL;

	PR_DISP_INFO("%s\n", __func__);
	if (!pdev) {
		pr_err("%s: invalid input\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = platform_get_drvdata(pdev);
	if (!ctrl_pdata) {
		pr_err("%s: invalid driver data\n", __func__);
		return -EINVAL;
	}

	pwrdata = devm_kzalloc(&pdev->dev,
				sizeof(struct dsi_power_data), GFP_KERNEL);
	if (!pwrdata) {
		pr_err("[DISP] %s: FAILED to alloc pwrdata\n", __func__);
		return -ENOMEM;
	}

	ctrl_pdata->dsi_pwrctrl_data = pwrdata;

	pwrdata->vddio = devm_regulator_get(&pdev->dev, "vddio");
	if (IS_ERR(pwrdata->vddio)) {
		pr_err("%s: could not get vddio reg, rc=%ld\n",
			__func__, PTR_ERR(pwrdata->vddio));
		return PTR_ERR(pwrdata->vddio);
	}

	ret = regulator_set_voltage(pwrdata->vddio, 1800000,
	        1800000);
	if (ret) {
		pr_err("%s: set voltage failed on vddio vreg, rc=%d\n",
			__func__, ret);
		return ret;
	}

	pwrdata->vdda = devm_regulator_get(&pdev->dev, "vdda");
	if (IS_ERR(pwrdata->vdda)) {
		pr_err("%s: could not get vdda vreg, rc=%ld\n",
			__func__, PTR_ERR(pwrdata->vdda));
		return PTR_ERR(pwrdata->vdda);
	}

	ret = regulator_set_voltage(pwrdata->vdda, 1200000,
	        1200000);
	if (ret) {
	    pr_err("%s: set voltage failed on vdda vreg, rc=%d\n",
	        __func__, ret);
	    return ret;
	}

	pwrdata->lcmio = of_get_named_gpio(pdev->dev.of_node,
						"htc,lcm_1v8-gpio", 0);
	pwrdata->lcmp5v = of_get_named_gpio(pdev->dev.of_node,
						"htc,lcm_p5v-gpio", 0);
	pwrdata->lcmn5v = of_get_named_gpio(pdev->dev.of_node,
						"htc,lcm_n5v-gpio", 0);
	pwrdata->lcm_bl_en = of_get_named_gpio(pdev->dev.of_node,
						"htc,lcm_bl_en-gpio", 0);

	if (gpio_is_valid(pwrdata->lcmio)) {
		/* EVM */
	} else {
		/* GLU(XA) and after */
		pwrdata->vlcmio = devm_regulator_get(&pdev->dev, "vlcmio");
		if (IS_ERR(pwrdata->vlcmio)) {
			pr_err("%s: could not get vlcmio reg, rc=%ld\n",
				__func__, PTR_ERR(pwrdata->vlcmio));
			return PTR_ERR(pwrdata->vlcmio);
		}
	}

	return 0;
}

static int htc_glu_regulator_deinit(struct platform_device *pdev)
{
	/* devm_regulator() will automatically free regulators
	   while dev detach. */
	/* nothing */
	return 0;
}

void htc_glu_panel_reset(struct mdss_panel_data *pdata, int enable)
{
	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;

	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);

	if (!gpio_is_valid(ctrl_pdata->disp_en_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
	}

	if (!gpio_is_valid(ctrl_pdata->rst_gpio)) {
		pr_debug("%s:%d, reset line not configured\n",
			   __func__, __LINE__);
		return;
	}

	pr_debug("%s: enable = %d\n", __func__, enable);

	if (enable) {
		if (pdata->panel_info.first_power_on == 1) {
			PR_DISP_INFO("reset already on in first time\n");
			return;
		}

		if (pdata->panel_info.panel_id == 1){
			gpio_set_value((ctrl_pdata->rst_gpio), 1);
			msleep(10);
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
			msleep(10);
			gpio_set_value((ctrl_pdata->rst_gpio), 1);
			msleep(10);
		} else {
			gpio_set_value((ctrl_pdata->rst_gpio), 0);
			msleep(10);
			gpio_set_value((ctrl_pdata->rst_gpio), 1);
			msleep(10);
		}

		if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
			gpio_set_value((ctrl_pdata->disp_en_gpio), 1);
		if (ctrl_pdata->ctrl_state & CTRL_STATE_PANEL_INIT) {
			pr_debug("%s: Panel Not properly turned OFF\n",
						__func__);
			ctrl_pdata->ctrl_state &= ~CTRL_STATE_PANEL_INIT;
			pr_debug("%s: Reset panel done\n", __func__);
		}
	} else {
		gpio_set_value((ctrl_pdata->rst_gpio), 0);
		if (gpio_is_valid(ctrl_pdata->disp_en_gpio))
			gpio_set_value((ctrl_pdata->disp_en_gpio), 0);
	}

}
static int htc_glu_panel_power_on(struct mdss_panel_data *pdata, int enable)
{
	int ret;

	struct mdss_dsi_ctrl_pdata *ctrl_pdata = NULL;
	struct dsi_power_data *pwrdata = NULL;

	PR_DISP_INFO("%s: en=%d\n", __func__, enable);
	if (pdata == NULL) {
		pr_err("%s: Invalid input data\n", __func__);
		return -EINVAL;
	}

	ctrl_pdata = container_of(pdata, struct mdss_dsi_ctrl_pdata,
				panel_data);
	pwrdata = ctrl_pdata->dsi_pwrctrl_data;

	if (!pwrdata) {
		pr_err("%s: pwrdata not initialized\n", __func__);
		return -EINVAL;
	}

	if (enable) {
		if (gpio_is_valid(pwrdata->lcmio)) {
			/* EVM */
			gpio_set_value(pwrdata->lcmio, 1);
		} else {
			ret = regulator_enable(pwrdata->vlcmio);
			if (ret) {
				pr_err("%s: Failed to enable regulator.\n",
					__func__);
				return ret;
			}
		}

		msleep(10);
		gpio_set_value(pwrdata->lcmp5v, 1);
		msleep(10);
		gpio_set_value(pwrdata->lcmn5v, 1);
		msleep(10);

		ret = regulator_set_optimum_mode(pwrdata->vddio, 100000);
		if (ret < 0) {
			pr_err("%s: vddio set opt mode failed.\n",
				__func__);
			return ret;
		}

		ret = regulator_set_optimum_mode(pwrdata->vdda, 100000);
		if (ret < 0) {
			pr_err("%s: vdda set opt mode failed.\n",
				__func__);
			return ret;
		}

		ret = regulator_enable(pwrdata->vddio);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n",
				__func__);
			return ret;
		}

		msleep(10);

		ret = regulator_enable(pwrdata->vdda);
		if (ret) {
			pr_err("%s: Failed to enable regulator.\n",
				__func__);
			return ret;
		}
		msleep(10);

		gpio_set_value(pwrdata->lcm_bl_en, 1);
	} else {
		gpio_set_value(pwrdata->lcm_bl_en, 0);
		msleep(65);

		if (pdata->panel_info.panel_id != 1)
			htc_glu_panel_reset(pdata, 0);

		ret = regulator_disable(pwrdata->vdda);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n",
				__func__);
			return ret;
		}

		ret = regulator_disable(pwrdata->vddio);
		if (ret) {
			pr_err("%s: Failed to disable regulator.\n",
				__func__);
			return ret;
		}

		ret = regulator_set_optimum_mode(pwrdata->vddio, 100);
		if (ret < 0) {
			pr_err("%s: vdd_io_vreg set opt mode failed.\n",
				__func__);
			return ret;
		}
		ret = regulator_set_optimum_mode(pwrdata->vdda, 100);
		if (ret < 0) {
			pr_err("%s: vdda_vreg set opt mode failed.\n",
				__func__);
			return ret;
		}
		msleep(10);
		gpio_set_value(pwrdata->lcmn5v, 0);
		msleep(10);
		gpio_set_value(pwrdata->lcmp5v, 0);

		if (pdata->panel_info.panel_id == 1){
			msleep(10);
			htc_glu_panel_reset(pdata, 0);
		}

		msleep(10);

		if (gpio_is_valid(pwrdata->lcmio)) {
			gpio_set_value(pwrdata->lcmio, 0);
		} else {
			ret = regulator_disable(pwrdata->vlcmio);
			if (ret) {
				pr_err("%s: Failed to enable regulator.\n",
					__func__);
				return ret;
			}
		}
	}
	PR_DISP_INFO("%s: en=%d done\n", __func__, enable);

	return 0;
}

static struct mdss_dsi_pwrctrl dsi_pwrctrl = {
	.dsi_regulator_init = htc_glu_regulator_init,
	.dsi_regulator_deinit = htc_glu_regulator_deinit,
	.dsi_power_on = htc_glu_panel_power_on,
	.dsi_panel_reset = htc_glu_panel_reset,
};

static struct platform_device dsi_pwrctrl_device = {
	.name          = "mdss_dsi_pwrctrl",
	.id            = -1,
	.dev.platform_data = &dsi_pwrctrl,
};

int __init htc_8974_dsi_panel_power_register(void)
{
	pr_info("%s#%d\n", __func__, __LINE__);
	platform_device_register(&dsi_pwrctrl_device);
	return 0;
}
