/* linux/arch/arm/mach-s3c2440/mach-osiris-dvs.c
 *
 * Copyright (c) 2009 Simtec Electronics
 *	http://armlinux.simtec.co.uk/
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Simtec Osiris Dynamic Voltage Scaling support.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/gpio.h>

#include <linux/i2c/tps65010.h>

#include <plat/cpu-freq.h>

#define OSIRIS_GPIO_DVS	S3C2410_GPB(5)

static bool dvs_en;

static void osiris_dvs_tps_setdvs(bool on)
{
	unsigned vregs1 = 0, vdcdc2 = 0;

	if (!on) {
		vdcdc2 = TPS_VCORE_DISCH | TPS_LP_COREOFF;
		vregs1 = TPS_LDO1_OFF;	/*                            */
	}

	dvs_en = on;
	vdcdc2 |= TPS_VCORE_1_3V | TPS_VCORE_LP_1_0V;
	vregs1 |= TPS_LDO2_ENABLE | TPS_LDO1_ENABLE;

	tps65010_config_vregs1(vregs1);
	tps65010_config_vdcdc2(vdcdc2);
}

static bool is_dvs(struct s3c_freq *f)
{
	/*                                               */
	return f->armclk == f->hclk;
}

/*                             */
static bool cur_dvs = false;

static int osiris_dvs_notify(struct notifier_block *nb,
			      unsigned long val, void *data)
{
	struct cpufreq_freqs *cf = data;
	struct s3c_cpufreq_freqs *freqs = to_s3c_cpufreq(cf);
	bool old_dvs = is_dvs(&freqs->old);
	bool new_dvs = is_dvs(&freqs->new);
	int ret = 0;

	if (!dvs_en)
		return 0;

	printk(KERN_DEBUG "%s: old %ld,%ld new %ld,%ld\n", __func__,
	       freqs->old.armclk, freqs->old.hclk,
	       freqs->new.armclk, freqs->new.hclk);

	switch (val) {
	case CPUFREQ_PRECHANGE:
		if (old_dvs & !new_dvs ||
		    cur_dvs & !new_dvs) {
			pr_debug("%s: exiting dvs\n", __func__);
			cur_dvs = false;
			gpio_set_value(OSIRIS_GPIO_DVS, 1);
		}
		break;
	case CPUFREQ_POSTCHANGE:
		if (!old_dvs & new_dvs ||
		    !cur_dvs & new_dvs) {
			pr_debug("entering dvs\n");
			cur_dvs = true;
			gpio_set_value(OSIRIS_GPIO_DVS, 0);
		}
		break;
	}

	return ret;
}

static struct notifier_block osiris_dvs_nb = {
	.notifier_call	= osiris_dvs_notify,
};

static int __devinit osiris_dvs_probe(struct platform_device *pdev)
{
	int ret;

	dev_info(&pdev->dev, "initialising\n");

	ret = gpio_request(OSIRIS_GPIO_DVS, "osiris-dvs");
	if (ret) {
		dev_err(&pdev->dev, "cannot claim gpio\n");
		goto err_nogpio;
	}

	/*                         */
	gpio_direction_output(OSIRIS_GPIO_DVS, 1);

	ret = cpufreq_register_notifier(&osiris_dvs_nb,
					CPUFREQ_TRANSITION_NOTIFIER);
	if (ret) {
		dev_err(&pdev->dev, "failed to register with cpufreq\n");
		goto err_nofreq;
	}

	osiris_dvs_tps_setdvs(true);

	return 0;

err_nofreq:
	gpio_free(OSIRIS_GPIO_DVS);

err_nogpio:
	return ret;
}

static int __devexit osiris_dvs_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "exiting\n");

	/*                         */
	gpio_set_value(OSIRIS_GPIO_DVS, 1);
	osiris_dvs_tps_setdvs(false);

	cpufreq_unregister_notifier(&osiris_dvs_nb,
				    CPUFREQ_TRANSITION_NOTIFIER);

	gpio_free(OSIRIS_GPIO_DVS);

	return 0;
}

/*                                                                      
                                       */

static int osiris_dvs_suspend(struct device *dev)
{
	gpio_set_value(OSIRIS_GPIO_DVS, 1);
	osiris_dvs_tps_setdvs(false);
	cur_dvs = false;

	return 0;
}

static int osiris_dvs_resume(struct device *dev)
{
	osiris_dvs_tps_setdvs(true);
	return 0;
}

static const struct dev_pm_ops osiris_dvs_pm = {
	.suspend	= osiris_dvs_suspend,
	.resume		= osiris_dvs_resume,
};

static struct platform_driver osiris_dvs_driver = {
	.probe		= osiris_dvs_probe,
	.remove		= __devexit_p(osiris_dvs_remove),
	.driver		= {
		.name	= "osiris-dvs",
		.owner	= THIS_MODULE,
		.pm	= &osiris_dvs_pm,
	},
};

static int __init osiris_dvs_init(void)
{
	return platform_driver_register(&osiris_dvs_driver);
}

static void __exit osiris_dvs_exit(void)
{
	platform_driver_unregister(&osiris_dvs_driver);
}

module_init(osiris_dvs_init);
module_exit(osiris_dvs_exit);

MODULE_DESCRIPTION("Simtec OSIRIS DVS support");
MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:osiris-dvs");
