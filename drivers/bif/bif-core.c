/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/bitrev.h>
#include <linux/crc-ccitt.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/workqueue.h>
#include <linux/bif/consumer.h>
#include <linux/bif/driver.h>

/* 
                                                                     
                                                         
                                              
                                                         
                                                     
                                                     
                                 
                                                           
                                                         
                                                              
                                    
                                                                    
                                               
                      
                                                                        
                                                 
                                              
                                                                 
                                                   
                                              
                
                                                                           
                                                
                                             
                                                   
                                                  
                           
                                                                    
                                                 
                                                   
              
 */
struct bif_ctrl_dev {
	struct list_head		list;
	struct bif_ctrl_desc		*desc;
	struct mutex			mutex;
	struct device			*ctrl_dev;
	void				*driver_data;
	struct bif_slave_dev		*selected_sdev;
	struct blocking_notifier_head	bus_change_notifier;
	struct delayed_work		enter_irq_mode_work;
	int				irq_count;
	int				irq_mode_delay_jiffies;
	bool				battery_present;
};

/* 
                                                                      
               
                                           
                                                                          
                                                   
                                                           
                                       
 */
struct bif_ctrl {
	struct bif_ctrl_dev	*bdev;
	bool			exclusive_lock;
};

/* 
                                                            
                                                         
                                          
                                                          
                                      
                                                          
                                                  
                                                                      
                                                      
                                                  
                                      
                                                     
                                                
                                                     
                                         
                  
                                                            
                    
                                                                     
                                          
                                                                     
                                                     
                             
                                                                        
                                                  
                                        
                                                                   
                                                   
                                                  
  
                                                                              
                                                                              
                                                  
  
                                                              
                                                 
 */
struct bif_slave_dev {
	struct list_head			list;
	struct bif_ctrl_dev			*bdev;
	u8					slave_addr;
	u8				unique_id[BIF_UNIQUE_ID_BYTE_LENGTH];
	int					unique_id_bits_known;
	bool					present;
	struct bif_ddb_l1_data			l1_data;
	struct bif_ddb_l2_data			*function_directory;
	struct bif_protocol_function		*protocol_function;
	struct bif_slave_control_function	*slave_ctrl_function;
	struct bif_nvm_function			*nvm_function;
};

/* 
                                                                         
               
                                              
                                      
 */
struct bif_slave {
	struct bif_ctrl				ctrl;
	struct bif_slave_dev			*sdev;
};

/*                                                                            */
#define BIF_TRANSACTION_RETRY_COUNT	5

static DEFINE_MUTEX(bif_ctrl_list_mutex);
static LIST_HEAD(bif_ctrl_list);
static DEFINE_MUTEX(bif_sdev_list_mutex);
static LIST_HEAD(bif_sdev_list);

static u8 next_dev_addr = 0x02;

#define DEBUG_PRINT_BUFFER_SIZE 256
static void fill_string(char *str, size_t str_len, u8 *buf, int buf_len)
{
	int pos = 0;
	int i;

	for (i = 0; i < buf_len; i++) {
		pos += scnprintf(str + pos, str_len - pos, "0x%02X", buf[i]);
		if (i < buf_len - 1)
			pos += scnprintf(str + pos, str_len - pos, ", ");
	}
}

static void bif_print_slave_data(struct bif_slave_dev *sdev)
{
	char str[DEBUG_PRINT_BUFFER_SIZE];
	u8 *uid;
	int i, j;
	struct bif_object *object;

	if (sdev->unique_id_bits_known != BIF_UNIQUE_ID_BIT_LENGTH)
		return;

	uid = sdev->unique_id;
	pr_debug("BIF slave: 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
		uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6],
		uid[7], uid[8], uid[9]);
	pr_debug("  present=%d, dev_adr=0x%02X\n", sdev->present,
		sdev->slave_addr);
	pr_debug("  revision=0x%02X, level=0x%02X, device class=0x%04X\n",
		sdev->l1_data.revision, sdev->l1_data.level,
		sdev->l1_data.device_class);
	pr_debug("  manufacturer ID=0x%04X, product ID=0x%04X\n",
		sdev->l1_data.manufacturer_id, sdev->l1_data.product_id);
	pr_debug("  function directory length=%d\n", sdev->l1_data.length);

	for (i = 0; i < sdev->l1_data.length / 4; i++) {
		pr_debug("  Function %d: type=0x%02X, version=0x%02X, pointer=0x%04X\n",
			i, sdev->function_directory[i].function_type,
			sdev->function_directory[i].function_version,
			sdev->function_directory[i].function_pointer);
	}

	if (sdev->nvm_function) {
		pr_debug("  NVM function: pointer=0x%04X, task=%d, wr_buf_size=%d, nvm_base=0x%04X, nvm_size=%d\n",
			sdev->nvm_function->nvm_pointer,
			sdev->nvm_function->slave_control_channel,
			(sdev->nvm_function->write_buffer_size
				? sdev->nvm_function->write_buffer_size : 0),
			sdev->nvm_function->nvm_base_address,
			sdev->nvm_function->nvm_size);
		if (sdev->nvm_function->object_count)
			pr_debug("  NVM objects:\n");
		i = 0;
		list_for_each_entry(object, &sdev->nvm_function->object_list,
					list) {
			pr_debug("    Object %d - addr=0x%04X, data len=%d, type=0x%02X, version=0x%02X, manufacturer ID=0x%04X, crc=0x%04X\n",
				i, object->addr, object->length - 8,
				object->type, object->version,
				object->manufacturer_id, object->crc);
			for (j = 0; j < DIV_ROUND_UP(object->length - 8, 16);
					j++) {
				fill_string(str, DEBUG_PRINT_BUFFER_SIZE,
					object->data + j * 16,
					min(16, object->length - 8 - (j * 16)));
				pr_debug("      data(0x%04X): %s\n", j * 16,
					str);
			}
			i++;
		}
	}
}

static void bif_print_slaves(void)
{
	struct bif_slave_dev *sdev;

	mutex_lock(&bif_sdev_list_mutex);

	list_for_each_entry(sdev, &bif_sdev_list, list) {
		/*                                       */
		if (sdev->unique_id_bits_known != BIF_UNIQUE_ID_BIT_LENGTH)
			continue;
		bif_print_slave_data(sdev);
	}

	mutex_unlock(&bif_sdev_list_mutex);
}

static struct bif_slave_dev *bif_add_slave(struct bif_ctrl_dev *bdev)
{
	struct bif_slave_dev *sdev;

	sdev = kzalloc(sizeof(struct bif_slave_dev), GFP_KERNEL);
	if (sdev == NULL) {
		pr_err("Memory allocation failed for bif_slave_dev\n");
		return ERR_PTR(-ENOMEM);
	}

	sdev->bdev = bdev;
	INIT_LIST_HEAD(&sdev->list);
	list_add_tail(&sdev->list, &bif_sdev_list);

	return sdev;
}

static void bif_remove_slave(struct bif_slave_dev *sdev)
{
	list_del(&sdev->list);
	if (sdev->bdev->selected_sdev == sdev)
		sdev->bdev->selected_sdev = NULL;

	if (sdev->slave_ctrl_function)
		kfree(sdev->slave_ctrl_function->irq_notifier_list);
	kfree(sdev->slave_ctrl_function);
	kfree(sdev->protocol_function);
	kfree(sdev->function_directory);

	kfree(sdev);
}

/*                                                                  */
static void set_uid_bit(u8 uid[BIF_UNIQUE_ID_BYTE_LENGTH], unsigned int bit,
			unsigned int value)
{
	u8 mask;

	if (bit >= BIF_UNIQUE_ID_BIT_LENGTH)
		return;

	mask = 1 << (7 - (bit % 8));

	uid[bit / 8] &= ~mask;
	uid[bit / 8] |= value << (7 - (bit % 8));
}

static unsigned int get_uid_bit(u8 uid[BIF_UNIQUE_ID_BYTE_LENGTH],
			unsigned int bit)
{
	if (bit >= BIF_UNIQUE_ID_BIT_LENGTH)
		return 0;

	return (uid[bit / 8] & (1 << (7 - (bit % 8)))) ? 1 : 0;
}

static void bif_enter_irq_mode_work(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct bif_ctrl_dev *bdev
		= container_of(dwork, struct bif_ctrl_dev, enter_irq_mode_work);
	int rc, i;

	mutex_lock(&bdev->mutex);
	for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
		rc = bdev->desc->ops->set_bus_state(bdev,
					BIF_BUS_STATE_INTERRUPT);
		if (rc == 0)
			break;
	}
	mutex_unlock(&bdev->mutex);

	/*                                                */
	if (rc) {
		pr_err("Could not set BIF bus to interrupt mode, rc=%d\n", rc);
		schedule_delayed_work(&bdev->enter_irq_mode_work,
					bdev->irq_mode_delay_jiffies);
	}
}

static void bif_cancel_irq_mode_work(struct bif_ctrl_dev *bdev)
{
	cancel_delayed_work(&bdev->enter_irq_mode_work);
}

static void bif_schedule_irq_mode_work(struct bif_ctrl_dev *bdev)
{
	if (bdev->irq_count > 0 &&
	    bdev->desc->ops->get_bus_state(bdev) != BIF_BUS_STATE_INTERRUPT)
		schedule_delayed_work(&bdev->enter_irq_mode_work,
					bdev->irq_mode_delay_jiffies);
}

static int _bif_select_slave_no_retry(struct bif_slave_dev *sdev)
{
	struct bif_ctrl_dev *bdev = sdev->bdev;
	int rc = 0;
	int i;

	/*                                         */
	if (sdev->bdev->selected_sdev == sdev)
		return 0;

	if (sdev->slave_addr) {
		/*                       */
		rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_SDA,
							sdev->slave_addr);
		if (!rc)
			sdev->bdev->selected_sdev = sdev;
	} else if (sdev->unique_id_bits_known == BIF_UNIQUE_ID_BIT_LENGTH) {
		/*                        */
		for (i = 0; i < BIF_UNIQUE_ID_BYTE_LENGTH - 1; i++) {
			rc = bdev->desc->ops->bus_transaction(bdev,
				BIF_TRANS_EDA, sdev->unique_id[i]);
			if (rc)
				goto out;
		}

		rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_SDA,
			sdev->unique_id[BIF_UNIQUE_ID_BYTE_LENGTH - 1]);
		if (rc)
			goto out;
	} else {
		pr_err("Cannot select slave because it has neither UID nor DEV_ADR.\n");
		return -EINVAL;
	}

	sdev->bdev->selected_sdev = sdev;

	return 0;
out:
	pr_err("bus_transaction failed, rc=%d\n", rc);
	return rc;
}

static int bif_select_slave(struct bif_slave_dev *sdev)
{
	int rc = -EPERM;
	int i;

	for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
		rc = _bif_select_slave_no_retry(sdev);
		if (rc == 0)
			break;
		/*                          */
		sdev->bdev->selected_sdev = NULL;
	}

	return rc;
}

/*
                                                                          
         
 */
static int bif_is_slave_selected(struct bif_ctrl_dev *bdev)
{
	int rc = -EPERM;
	int tack, i;

	for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
		/*                              */
		rc = bdev->desc->ops->bus_transaction_read(bdev, BIF_TRANS_BC,
						BIF_CMD_TQ, &tack);
		if (rc == 0 || rc == -ETIMEDOUT)
			break;
	}

	if (rc == 0)
		rc = 1;
	else if (rc == -ETIMEDOUT)
		rc = 0;
	else
		pr_err("BIF bus_transaction_read failed, rc=%d\n", rc);

	return rc;
}

/*                                                        */
static int _bif_slave_read_no_retry(struct bif_slave_dev *sdev, u16 addr,
			u8 *buf, int len)
{
	struct bif_ctrl_dev *bdev = sdev->bdev;
	int rc = 0;
	int i, response;

	rc = bif_select_slave(sdev);
	if (rc)
		return rc;

	if (bdev->desc->ops->read_slave_registers) {
		/*
                                                                 
                                             
   */
		rc = bdev->desc->ops->read_slave_registers(bdev, addr, buf,
							   len);
		if (rc)
			pr_err("read_slave_registers failed, rc=%d\n", rc);
		return rc;
	}

	for (i = 0; i < len; i++) {
		rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_ERA,
							addr >> 8);
		if (rc) {
			pr_err("bus_transaction failed, rc=%d\n", rc);
			return rc;
		}

		rc = bdev->desc->ops->bus_transaction_read(bdev, BIF_TRANS_RRA,
							addr & 0xFF, &response);
		if (rc) {
			pr_err("bus_transaction_read failed, rc=%d\n", rc);
			return rc;
		}

		if (!(response & BIF_SLAVE_RD_ACK)) {
			pr_err("BIF register read error=0x%02X\n",
				response & BIF_SLAVE_RD_ERR);
			return -EIO;
		}

		buf[i] = response & BIF_SLAVE_RD_DATA;
		addr++;
	}

	return rc;
}

/*
                                                                                
                                                  
 */
static int _bif_slave_read(struct bif_slave_dev *sdev, u16 addr, u8 *buf,
			int len)
{
	int rc = -EPERM;
	int i;

	for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
		rc = _bif_slave_read_no_retry(sdev, addr, buf, len);
		if (rc == 0)
			break;
		/*                          */
		sdev->bdev->selected_sdev = NULL;
	}

	return rc;
}

/*                                                       */
static int _bif_slave_write_no_retry(struct bif_slave_dev *sdev, u16 addr,
			u8 *buf, int len)
{
	struct bif_ctrl_dev *bdev = sdev->bdev;
	int rc = 0;
	int i;

	rc = bif_select_slave(sdev);
	if (rc)
		return rc;

	if (bdev->desc->ops->write_slave_registers) {
		/*
                                                                  
                                              
   */
		rc = bdev->desc->ops->write_slave_registers(bdev, addr, buf,
							    len);
		if (rc)
			pr_err("write_slave_registers failed, rc=%d\n", rc);
		return rc;
	}

	rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_ERA, addr >> 8);
	if (rc)
		goto out;

	rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_WRA, addr & 0xFF);
	if (rc)
		goto out;

	for (i = 0; i < len; i++) {
		rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_WD,
							buf[i]);
		if (rc)
			goto out;
	}

	return 0;
out:
	pr_err("bus_transaction failed, rc=%d\n", rc);
	return rc;
}

/*
                                                                               
                                                  
 */
static int _bif_slave_write(struct bif_slave_dev *sdev, u16 addr, u8 *buf,
			int len)
{
	int rc = -EPERM;
	int i;

	for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
		rc = _bif_slave_write_no_retry(sdev, addr, buf, len);
		if (rc == 0)
			break;
		/*                          */
		sdev->bdev->selected_sdev = NULL;
	}

	return rc;
}

/*                                                              */
static void bif_ctrl_lock(struct bif_ctrl *ctrl)
{
	if (!ctrl->exclusive_lock) {
		mutex_lock(&ctrl->bdev->mutex);
		bif_cancel_irq_mode_work(ctrl->bdev);
	}
}

/*                                                                 */
static void bif_ctrl_unlock(struct bif_ctrl *ctrl)
{
	if (!ctrl->exclusive_lock) {
		bif_schedule_irq_mode_work(ctrl->bdev);
		mutex_unlock(&ctrl->bdev->mutex);
	}
}

static void bif_slave_ctrl_lock(struct bif_slave *slave)
{
	bif_ctrl_lock(&slave->ctrl);
}

static void bif_slave_ctrl_unlock(struct bif_slave *slave)
{
	bif_ctrl_unlock(&slave->ctrl);
}

static int bif_check_task(struct bif_slave *slave, unsigned int task)
{
	if (IS_ERR_OR_NULL(slave)) {
		pr_err("Invalid slave handle.\n");
		return -EINVAL;
	} else if (!slave->sdev->bdev) {
		pr_err("BIF controller has been removed.\n");
		return -ENXIO;
	} else if (!slave->sdev->slave_ctrl_function
			|| slave->sdev->slave_ctrl_function->task_count == 0) {
		pr_err("BIF slave does not support slave control.\n");
		return -ENODEV;
	} else if (task >= slave->sdev->slave_ctrl_function->task_count) {
		pr_err("Requested task: %u greater than max: %u for this slave\n",
			task, slave->sdev->slave_ctrl_function->task_count);
		return -EINVAL;
	}

	return 0;
}

/* 
                                                                   
                           
                                                               
                                                                  
                                     
                                                  
  
                                                                                
                                                                                
                                                                             
                  
  
                                                       
 */
int bif_request_irq(struct bif_slave *slave, unsigned int task,
			struct notifier_block *nb)
{
	int rc;
	u16 addr;
	u8 reg, mask;

	rc = bif_check_task(slave, task);
	if (rc) {
		pr_err("Invalid slave or task, rc=%d\n", rc);
		return rc;
	}

	bif_slave_ctrl_lock(slave);

	rc = blocking_notifier_chain_register(
		&slave->sdev->slave_ctrl_function->irq_notifier_list[task], nb);
	if (rc) {
		pr_err("Notifier registration failed, rc=%d\n", rc);
		goto done;
	}

	/*                                       */
	mask = BIT(task % SLAVE_CTRL_TASKS_PER_SET);
	addr = SLAVE_CTRL_FUNC_IRQ_EN_ADDR(
		slave->sdev->slave_ctrl_function->slave_ctrl_pointer, task);
	if (task / SLAVE_CTRL_TASKS_PER_SET == 0) {
		/*                              */
		mask |= BIT(0);
	}
	rc = _bif_slave_read(slave->sdev, addr, &reg, 1);
	if (rc) {
		pr_err("BIF slave register read failed, rc=%d\n", rc);
		goto notifier_unregister;
	}
	reg |= mask;
	rc = _bif_slave_write(slave->sdev, addr, &reg, 1);
	if (rc) {
		pr_err("BIF slave register write failed, rc=%d\n", rc);
		goto notifier_unregister;
	}

	/*                                                   */
	if (task / SLAVE_CTRL_TASKS_PER_SET != 0) {
		mask = BIT(0);
		addr = SLAVE_CTRL_FUNC_IRQ_EN_ADDR(
		       slave->sdev->slave_ctrl_function->slave_ctrl_pointer, 0);
		rc = _bif_slave_read(slave->sdev, addr, &reg, 1);
		if (rc) {
			pr_err("BIF slave register read failed, rc=%d\n", rc);
			goto notifier_unregister;
		}
		reg |= mask;
		rc = _bif_slave_write(slave->sdev, addr, &reg, 1);
		if (rc) {
			pr_err("BIF slave register write failed, rc=%d\n", rc);
			goto notifier_unregister;
		}
	}

	rc = slave->sdev->bdev->desc->ops->set_bus_state(slave->sdev->bdev,
		BIF_BUS_STATE_INTERRUPT);
	if (rc) {
		pr_err("Could not set BIF bus to interrupt mode, rc=%d\n", rc);
		goto notifier_unregister;
	}

	slave->sdev->bdev->irq_count++;
done:
	bif_slave_ctrl_unlock(slave);

	return rc;

notifier_unregister:
	blocking_notifier_chain_unregister(
		&slave->sdev->slave_ctrl_function->irq_notifier_list[task],
		nb);
	bif_slave_ctrl_unlock(slave);

	return rc;

}
EXPORT_SYMBOL(bif_request_irq);

/* 
                                                             
                           
                                                               
                                                                  
                                     
                                                                 
  
                                                                            
                          
  
                                                       
 */
int bif_free_irq(struct bif_slave *slave, unsigned int task,
			struct notifier_block *nb)
{
	int rc;
	u16 addr;
	u8 reg;

	rc = bif_check_task(slave, task);
	if (rc) {
		pr_err("Invalid slave or task, rc=%d\n", rc);
		return rc;
	}

	bif_slave_ctrl_lock(slave);

	/*                                        */
	reg = BIT(task % SLAVE_CTRL_TASKS_PER_SET);
	addr = SLAVE_CTRL_FUNC_IRQ_CLEAR_ADDR(
		slave->sdev->slave_ctrl_function->slave_ctrl_pointer, task);
	rc = _bif_slave_write(slave->sdev, addr, &reg, 1);
	if (rc) {
		pr_err("BIF slave register write failed, rc=%d\n", rc);
		goto done;
	}

	rc = blocking_notifier_chain_unregister(
		&slave->sdev->slave_ctrl_function->irq_notifier_list[task], nb);
	if (rc) {
		pr_err("Notifier unregistration failed, rc=%d\n", rc);
		goto done;
	}

	slave->sdev->bdev->irq_count--;

	if (slave->sdev->bdev->irq_count == 0) {
		bif_cancel_irq_mode_work(slave->sdev->bdev);
	} else if (slave->sdev->bdev->irq_count < 0) {
		pr_err("Unbalanced IRQ free.\n");
		rc = -EINVAL;
		slave->sdev->bdev->irq_count = 0;
	}
done:
	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_free_irq);

/* 
                                                         
                           
                                                                       
                                                                
                        
  
                                                       
 */
int bif_trigger_task(struct bif_slave *slave, unsigned int task)
{
	int rc;
	u16 addr;
	u8 reg;

	rc = bif_check_task(slave, task);
	if (rc) {
		pr_err("Invalid slave or task, rc=%d\n", rc);
		return rc;
	}

	bif_slave_ctrl_lock(slave);

	/*                                    */
	reg = BIT(task % SLAVE_CTRL_TASKS_PER_SET);
	addr = SLAVE_CTRL_FUNC_TASK_TRIGGER_ADDR(
		slave->sdev->slave_ctrl_function->slave_ctrl_pointer, task);
	rc = _bif_slave_write(slave->sdev, addr, &reg, 1);
	if (rc) {
		pr_err("BIF slave register write failed, rc=%d\n", rc);
		goto done;
	}

done:
	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_trigger_task);

/* 
                                                            
                           
                                                                       
                                                                
                        
  
                                                                          
 */
int bif_task_is_busy(struct bif_slave *slave, unsigned int task)
{
	int rc;
	u16 addr;
	u8 reg;

	rc = bif_check_task(slave, task);
	if (rc) {
		pr_err("Invalid slave or task, rc=%d\n", rc);
		return rc;
	}

	bif_slave_ctrl_lock(slave);

	/*                            */
	addr = SLAVE_CTRL_FUNC_TASK_BUSY_ADDR(
		slave->sdev->slave_ctrl_function->slave_ctrl_pointer, task);
	rc = _bif_slave_read(slave->sdev, addr, &reg, 1);
	if (rc) {
		pr_err("BIF slave register read failed, rc=%d\n", rc);
		goto done;
	}

	rc = (reg & BIT(task % SLAVE_CTRL_TASKS_PER_SET)) ? 1 : 0;
done:
	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_task_is_busy);

static int bif_slave_notify_irqs(struct bif_slave_dev *sdev, int set, u8 val)
{
	int rc = 0;
	int i, task;

	for (i = 0; i < SLAVE_CTRL_TASKS_PER_SET; i++) {
		if (val & (1 << i)) {
			task = set * SLAVE_CTRL_TASKS_PER_SET + i;

			rc = blocking_notifier_call_chain(
			    &sdev->slave_ctrl_function->irq_notifier_list[task],
			    task, sdev->bdev);
			rc = notifier_to_errno(rc);
			if (rc)
				pr_err("Notification failed for task %d\n",
					task);
		}
	}

	return rc;
}

static int bif_slave_handle_irq(struct bif_slave_dev *sdev)
{
	struct bif_ctrl_dev *bdev = sdev->bdev;
	bool resp = false;
	int rc = 0;
	int i;
	u16 addr;
	u8 reg;

	mutex_lock(&sdev->bdev->mutex);
	bif_cancel_irq_mode_work(sdev->bdev);

	rc = bif_select_slave(sdev);
	if (rc) {
		pr_err("Could not select slave, rc=%d\n", rc);
		goto done;
	}

	/*                                       */
	rc = bdev->desc->ops->bus_transaction_query(bdev, BIF_TRANS_BC,
						    BIF_CMD_ISTS, &resp);
	if (rc) {
		pr_err("Could not query slave interrupt status, rc=%d\n", rc);
		goto done;
	}

	if (resp) {
		for (i = 0; i < sdev->slave_ctrl_function->task_count
					/ SLAVE_CTRL_TASKS_PER_SET; i++) {
			addr = sdev->slave_ctrl_function->slave_ctrl_pointer
				+ 4 * i + 1;
			rc = _bif_slave_read(sdev, addr, &reg, 1);
			if (rc) {
				pr_err("BIF slave register read failed, rc=%d\n",
					rc);
				goto done;
			}

			/*                                                */
			if (reg != 0x00) {
				/*
                                                  
                                 
     */
				mutex_unlock(&sdev->bdev->mutex);
				rc = bif_slave_notify_irqs(sdev, i, reg);
				if (rc) {
					pr_err("BIF slave irq notification failed, rc=%d\n",
						rc);
					goto notification_failed;
				}
				mutex_lock(&sdev->bdev->mutex);

				rc = bif_select_slave(sdev);
				if (rc) {
					pr_err("Could not select slave, rc=%d\n",
						rc);
					goto done;
				}

				/*                                   */
				rc = _bif_slave_write(sdev, addr, &reg, 1);
				if (rc) {
					pr_err("BIF slave register write failed, rc=%d\n",
						rc);
					goto done;
				}
			}
		}
	}

done:
	bif_schedule_irq_mode_work(sdev->bdev);
	mutex_unlock(&sdev->bdev->mutex);
notification_failed:
	if (rc == 0)
		rc = resp;
	return rc;
}

/* 
                                                                                
                                      
                                       
  
                                                                    
  
                                                       
 */
int bif_ctrl_notify_slave_irq(struct bif_ctrl_dev *bdev)
{
	struct bif_slave_dev *sdev;
	int rc = 0, handled = 0;

	if (IS_ERR_OR_NULL(bdev))
		return -EINVAL;

	mutex_lock(&bif_sdev_list_mutex);

	list_for_each_entry(sdev, &bif_sdev_list, list) {
		if (sdev->bdev == bdev && sdev->present) {
			rc = bif_slave_handle_irq(sdev);
			if (rc < 0) {
				pr_err("Could not handle BIF slave irq, rc=%d\n",
					rc);
				break;
			}
			handled += rc;
		}
	}

	mutex_unlock(&bif_sdev_list_mutex);

	if (handled == 0)
		pr_info("Spurious BIF slave interrupt detected.\n");

	if (rc > 0)
		rc = 0;

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_notify_slave_irq);

/* 
                                                                              
                                       
                                       
  
                                                                    
  
                                                       
 */
int bif_ctrl_notify_battery_changed(struct bif_ctrl_dev *bdev)
{
	int rc = 0;
	int present;

	if (IS_ERR_OR_NULL(bdev))
		return -EINVAL;

	if (bdev->desc->ops->get_battery_presence) {
		present = bdev->desc->ops->get_battery_presence(bdev);
		if (present < 0) {
			pr_err("Could not determine battery presence, rc=%d\n",
				rc);
			return rc;
		}

		if (bdev->battery_present == !!present)
			return 0;

		bdev->battery_present = present;

		rc = blocking_notifier_call_chain(&bdev->bus_change_notifier,
			present ? BIF_BUS_EVENT_BATTERY_INSERTED
				: BIF_BUS_EVENT_BATTERY_REMOVED, bdev);
		if (rc)
			pr_err("Call chain noification failed, rc=%d\n", rc);
	}

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_notify_battery_changed);

/* 
                                                                              
                                       
                                        
  
                                                                                
                                                                              
              
  
                                                       
 */
int bif_ctrl_signal_battery_changed(struct bif_ctrl *ctrl)
{
	if (IS_ERR_OR_NULL(ctrl))
		return -EINVAL;

	return bif_ctrl_notify_battery_changed(ctrl->bdev);
}
EXPORT_SYMBOL(bif_ctrl_signal_battery_changed);

/* 
                                                                             
                                           
                                        
  
                                                                 
                      
  
                                                       
 */
int bif_ctrl_notifier_register(struct bif_ctrl *ctrl, struct notifier_block *nb)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl))
		return -EINVAL;

	rc = blocking_notifier_chain_register(&ctrl->bdev->bus_change_notifier,
					      nb);
	if (rc)
		pr_err("Notifier registration failed, rc=%d\n", rc);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_notifier_register);

/* 
                                                                               
                                          
                                        
  
                                                       
 */
int bif_ctrl_notifier_unregister(struct bif_ctrl *ctrl,
				 struct notifier_block *nb)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl))
		return -EINVAL;

	rc =
	    blocking_notifier_chain_unregister(&ctrl->bdev->bus_change_notifier,
						nb);
	if (rc)
		pr_err("Notifier unregistration failed, rc=%d\n", rc);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_notifier_unregister);

/* 
                                                                               
                            
                           
  
                                                                        
                        
 */
struct bif_ctrl *bif_get_bus_handle(struct bif_slave *slave)
{
	if (IS_ERR_OR_NULL(slave))
		return ERR_PTR(-EINVAL);

	return &slave->ctrl;
}
EXPORT_SYMBOL(bif_get_bus_handle);

/* 
                                                                      
 */
int bif_ctrl_count(void)
{
	struct bif_ctrl_dev *bdev;
	int count = 0;

	mutex_lock(&bif_ctrl_list_mutex);

	list_for_each_entry(bdev, &bif_ctrl_list, list) {
		count++;
	}
	mutex_unlock(&bif_ctrl_list_mutex);

	return count;
}
EXPORT_SYMBOL(bif_ctrl_count);

/* 
                                                                              
                  
                                                                  
  
                                                                                
                                                                            
                                        
  
                                                                               
 */
struct bif_ctrl *bif_ctrl_get_by_id(unsigned int id)
{
	struct bif_ctrl_dev *bdev;
	struct bif_ctrl_dev *bdev_found = NULL;
	struct bif_ctrl *ctrl = ERR_PTR(-ENODEV);

	mutex_lock(&bif_ctrl_list_mutex);

	list_for_each_entry(bdev, &bif_ctrl_list, list) {
		if (id == 0) {
			bdev_found = bdev;
			break;
		}
		id--;
	}
	mutex_unlock(&bif_ctrl_list_mutex);

	if (bdev_found) {
		ctrl = kzalloc(sizeof(*ctrl), GFP_KERNEL);
		if (!ctrl) {
			pr_err("Bus handle allocation failed\n");
			ctrl = ERR_PTR(-ENOMEM);
		} else {
			ctrl->bdev = bdev_found;
		}
	}

	return ctrl;
}
EXPORT_SYMBOL(bif_ctrl_get_by_id);

/* 
                                                                             
                                       
                                                  
  
                                                                            
                                                                         
                          
  
                                                                               
                                                                               
                                      
 */
struct bif_ctrl *bif_ctrl_get(struct device *consumer_dev)
{
	struct device_node *ctrl_node = NULL;
	struct bif_ctrl_dev *bdev_found = NULL;
	struct bif_ctrl *ctrl = ERR_PTR(-EPROBE_DEFER);
	struct bif_ctrl_dev *bdev = NULL;

	if (!consumer_dev || !consumer_dev->of_node) {
		pr_err("Invalid device node\n");
		return ERR_PTR(-EINVAL);
	}

	ctrl_node = of_parse_phandle(consumer_dev->of_node, "qcom,bif-ctrl", 0);
	if (!ctrl_node) {
		pr_err("Could not find qcom,bif-ctrl property in %s\n",
			consumer_dev->of_node->full_name);
		return ERR_PTR(-ENXIO);
	}

	mutex_lock(&bif_ctrl_list_mutex);
	list_for_each_entry(bdev, &bif_ctrl_list, list) {
		if (bdev->ctrl_dev && bdev->ctrl_dev->of_node == ctrl_node) {
			bdev_found = bdev;
			break;
		}
	}
	mutex_unlock(&bif_ctrl_list_mutex);

	if (bdev_found) {
		ctrl = kzalloc(sizeof(*ctrl), GFP_KERNEL);
		if (!ctrl) {
			pr_err("Bus handle allocation failed\n");
			ctrl = ERR_PTR(-ENOMEM);
		} else {
			ctrl->bdev = bdev_found;
		}
	}

	return ctrl;
}
EXPORT_SYMBOL(bif_ctrl_get);

/* 
                                                 
                                        
 */
void bif_ctrl_put(struct bif_ctrl *ctrl)
{
	if (!IS_ERR_OR_NULL(ctrl) && ctrl->exclusive_lock)
		mutex_unlock(&ctrl->bdev->mutex);
	kfree(ctrl);
}
EXPORT_SYMBOL(bif_ctrl_put);

/*
                                                               
                                                                             
                                                                               
                 
 */
static bool bif_slave_match(const struct bif_ctrl *ctrl,
	struct bif_slave_dev *sdev, const struct bif_match_criteria *criteria)
{
	int i, type, version;

	if (ctrl && (ctrl->bdev != sdev->bdev))
		return false;

	if (!sdev->present
	    && (!(criteria->match_mask & BIF_MATCH_IGNORE_PRESENCE)
		|| ((criteria->match_mask & BIF_MATCH_IGNORE_PRESENCE)
		    && !criteria->ignore_presence)))
		return false;

	if ((criteria->match_mask & BIF_MATCH_MANUFACTURER_ID)
	    && sdev->l1_data.manufacturer_id != criteria->manufacturer_id)
		return false;

	if ((criteria->match_mask & BIF_MATCH_PRODUCT_ID)
	    && sdev->l1_data.product_id != criteria->product_id)
		return false;

	if (criteria->match_mask & BIF_MATCH_FUNCTION_TYPE) {
		if (!sdev->function_directory)
			return false;
		for (i = 0; i < sdev->l1_data.length / 4; i++) {
			type = sdev->function_directory[i].function_type;
			version = sdev->function_directory[i].function_version;
			if (type == criteria->function_type &&
				(version == criteria->function_version
					|| !(criteria->match_mask
						& BIF_MATCH_FUNCTION_VERSION)))
				return true;
		}
		return false;
	}

	return true;
}

/* 
                                                                             
                                                    
             
                                         
                                                           
 */
int bif_slave_match_count(const struct bif_ctrl *ctrl,
			const struct bif_match_criteria *match_criteria)
{
	struct bif_slave_dev *sdev;
	int count = 0;

	mutex_lock(&bif_sdev_list_mutex);

	list_for_each_entry(sdev, &bif_sdev_list, list) {
		if (bif_slave_match(ctrl, sdev, match_criteria))
			count++;
	}

	mutex_unlock(&bif_sdev_list_mutex);

	return count;
}
EXPORT_SYMBOL(bif_slave_match_count);

/* 
                                                                            
                                                     
                      
                                         
                                               
                                                           
  
                                                                                
  
                                                                 
 */
struct bif_slave *bif_slave_match_get(const struct bif_ctrl *ctrl,
	unsigned int id, const struct bif_match_criteria *match_criteria)
{
	struct bif_slave_dev *sdev;
	struct bif_slave *slave = ERR_PTR(-ENODEV);
	struct bif_slave_dev *sdev_found = NULL;
	int count = 0;

	mutex_lock(&bif_sdev_list_mutex);

	list_for_each_entry(sdev, &bif_sdev_list, list) {
		if (bif_slave_match(ctrl, sdev, match_criteria))
			count++;
		if (count == id + 1) {
			sdev_found = sdev;
			break;
		}
	}

	mutex_unlock(&bif_sdev_list_mutex);

	if (sdev_found) {
		slave = kzalloc(sizeof(*slave), GFP_KERNEL);
		if (!slave) {
			pr_err("Slave allocation failed\n");
			slave = ERR_PTR(-ENOMEM);
		} else {
			slave->sdev = sdev_found;
			slave->ctrl.bdev = sdev_found->bdev;
		}
	}

	return slave;
}
EXPORT_SYMBOL(bif_slave_match_get);

/* 
                                             
                           
 */
void bif_slave_put(struct bif_slave *slave)
{
	if (!IS_ERR_OR_NULL(slave) && slave->ctrl.exclusive_lock)
		mutex_unlock(&slave->sdev->bdev->mutex);
	kfree(slave);
}
EXPORT_SYMBOL(bif_slave_put);

/* 
                                                                        
                                                         
                            
                                                             
                                                                    
                                  
                                                                              
                                             
  
                                                                               
                                                
 */
int bif_slave_find_function(struct bif_slave *slave, u8 function, u8 *version,
				u16 *function_pointer)
{
	int rc = -ENODEV;
	struct bif_ddb_l2_data *func;
	int i;

	if (IS_ERR_OR_NULL(slave) || IS_ERR_OR_NULL(version)
	    || IS_ERR_OR_NULL(function_pointer)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	func = slave->sdev->function_directory;

	for (i = 0; i < slave->sdev->l1_data.length / 4; i++) {
		if (function == func[i].function_type) {
			*version = func[i].function_version;
			*function_pointer = func[i].function_pointer;
			rc = 0;
			break;
		}
	}

	return rc;
}
EXPORT_SYMBOL(bif_slave_find_function);

/* 
                                                                    
                           
                                               
                                          
                               
  
                                                       
 */
int bif_slave_read(struct bif_slave *slave, u16 addr, u8 *buf, int len)
{
	int rc;

	if (IS_ERR_OR_NULL(slave) || IS_ERR_OR_NULL(buf)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	bif_slave_ctrl_lock(slave);

	rc = _bif_slave_read(slave->sdev, addr, buf, len);
	if (rc)
		pr_err("BIF slave read failed, rc=%d\n", rc);

	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_slave_read);

/* 
                                                                    
                           
                                               
                                          
                                
  
                                                       
 */
int bif_slave_write(struct bif_slave *slave, u16 addr, u8 *buf, int len)
{
	int rc;

	if (IS_ERR_OR_NULL(slave) || IS_ERR_OR_NULL(buf)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	bif_slave_ctrl_lock(slave);

	rc = _bif_slave_write(slave->sdev, addr, buf, len);
	if (rc)
		pr_err("BIF slave write failed, rc=%d\n", rc);

	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_slave_write);

/* 
                                                                            
                 
                           
  
                                                                             
                        
  
                                                                            
                                                          
 */
int bif_slave_is_present(struct bif_slave *slave)
{
	if (IS_ERR_OR_NULL(slave)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	return slave->sdev->present;
}
EXPORT_SYMBOL(bif_slave_is_present);

/* 
                                                                              
       
                           
  
                                                                               
                        
  
                                                                            
                                                                          
                                                                    
                                                         
 */
int bif_slave_is_selected(struct bif_slave *slave)
{
	int rc;

	if (IS_ERR_OR_NULL(slave)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	if (slave->sdev->bdev->selected_sdev != slave->sdev)
		return false;

	bif_slave_ctrl_lock(slave);
	rc = bif_is_slave_selected(slave->sdev->bdev);
	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_slave_is_selected);

/* 
                                                     
                           
  
                                                      
  
                                                                            
                                                                          
                                                                    
                                                         
 */
int bif_slave_select(struct bif_slave *slave)
{
	int rc;

	if (IS_ERR_OR_NULL(slave)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	bif_slave_ctrl_lock(slave);
	slave->sdev->bdev->selected_sdev = NULL;
	rc = bif_select_slave(slave->sdev);
	bif_slave_ctrl_unlock(slave);

	return rc;
}
EXPORT_SYMBOL(bif_slave_select);

/* 
                                                                              
                              
                                         
                                                                         
                                    
                                                                
                                                         
               
  
                                                                 
                                                                               
                                                        
  
                                                      
  
                                                                               
                                                 
 */
int bif_ctrl_raw_transaction(struct bif_ctrl *ctrl, int transaction, u8 data)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	bif_ctrl_lock(ctrl);

	rc = ctrl->bdev->desc->ops->bus_transaction(ctrl->bdev, transaction,
							data);
	if (rc)
		pr_err("BIF bus transaction failed, rc=%d\n", rc);

	bif_ctrl_unlock(ctrl);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_raw_transaction);

/* 
                                                                             
                                                    
                                         
                                                                         
                                    
                                                                
                                                         
               
                                                                    
                                                            
                                       
  
                                                                 
                                                                               
                                                        
  
                                                      
  
                                                                               
                                                 
 */
int bif_ctrl_raw_transaction_read(struct bif_ctrl *ctrl, int transaction,
					u8 data, int *response)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl) || IS_ERR_OR_NULL(response)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	bif_ctrl_lock(ctrl);

	rc = ctrl->bdev->desc->ops->bus_transaction_read(ctrl->bdev,
					transaction, data, response);
	if (rc)
		pr_err("BIF bus transaction failed, rc=%d\n", rc);

	bif_ctrl_unlock(ctrl);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_raw_transaction_read);

/* 
                                                                              
                                      
                                         
                                                                         
                                    
                                                                
                                                         
               
                                                                         
                                                             
                
  
                                                                 
                                                                               
                                                        
  
                                                      
  
                                                                               
                                                 
 */
int bif_ctrl_raw_transaction_query(struct bif_ctrl *ctrl, int transaction,
		u8 data, bool *query_response)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl) || IS_ERR_OR_NULL(query_response)) {
		pr_err("Invalid pointer input.\n");
		return -EINVAL;
	}

	bif_ctrl_lock(ctrl);

	rc = ctrl->bdev->desc->ops->bus_transaction_query(ctrl->bdev,
					transaction, data, query_response);
	if (rc)
		pr_err("BIF bus transaction failed, rc=%d\n", rc);

	bif_ctrl_unlock(ctrl);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_raw_transaction_query);

/* 
                                                                              
                                        
  
                                                                           
                                                                         
                
 */
void bif_ctrl_bus_lock(struct bif_ctrl *ctrl)
{
	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return;
	}

	if (ctrl->exclusive_lock) {
		pr_err("BIF bus exclusive lock already held\n");
		return;
	}

	mutex_lock(&ctrl->bdev->mutex);
	ctrl->exclusive_lock = true;
	bif_cancel_irq_mode_work(ctrl->bdev);
}
EXPORT_SYMBOL(bif_ctrl_bus_lock);

/* 
                                                                               
                               
                                        
  
                                                                             
 */
void bif_ctrl_bus_unlock(struct bif_ctrl *ctrl)
{
	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return;
	}

	if (!ctrl->exclusive_lock) {
		pr_err("BIF bus exclusive lock not already held\n");
		return;
	}

	ctrl->exclusive_lock = false;
	bif_schedule_irq_mode_work(ctrl->bdev);
	mutex_unlock(&ctrl->bdev->mutex);
}
EXPORT_SYMBOL(bif_ctrl_bus_unlock);

/* 
                                                                             
           
                                        
  
                                                                            
                        
 */
int bif_ctrl_measure_rid(struct bif_ctrl *ctrl)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return -ENODEV;
	}

	if (!ctrl->bdev->desc->ops->get_battery_rid) {
		pr_err("Cannot measure Rid.\n");
		return -ENXIO;
	}

	bif_ctrl_lock(ctrl);

	rc = ctrl->bdev->desc->ops->get_battery_rid(ctrl->bdev);
	if (rc < 0)
		pr_err("Error during Rid measurement, rc=%d\n", rc);

	bif_ctrl_unlock(ctrl);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_measure_rid);

/* 
                                                                              
                                        
  
                                                                              
                              
 */
int bif_ctrl_get_bus_period(struct bif_ctrl *ctrl)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return -ENODEV;
	}

	if (!ctrl->bdev->desc->ops->get_bus_period) {
		pr_err("Cannot get the BIF bus period.\n");
		return -ENXIO;
	}

	rc = ctrl->bdev->desc->ops->get_bus_period(ctrl->bdev);
	if (rc < 0)
		pr_err("Error during bus period retrieval, rc=%d\n", rc);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_get_bus_period);

/* 
                                                                              
                                        
                                                   
  
                                                                                
                                             
  
                                                      
 */
int bif_ctrl_set_bus_period(struct bif_ctrl *ctrl, int period_ns)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return -ENODEV;
	}

	if (!ctrl->bdev->desc->ops->set_bus_period) {
		pr_err("Cannot set the BIF bus period.\n");
		return -ENXIO;
	}

	bif_ctrl_lock(ctrl);
	rc = ctrl->bdev->desc->ops->set_bus_period(ctrl->bdev, period_ns);
	if (rc)
		pr_err("Error during bus period configuration, rc=%d\n", rc);
	bif_ctrl_unlock(ctrl);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_set_bus_period);

/* 
                                                                  
                                        
  
                                                                           
                  
 */
int bif_ctrl_get_bus_state(struct bif_ctrl *ctrl)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return -ENODEV;
	}

	rc = ctrl->bdev->desc->ops->get_bus_state(ctrl->bdev);
	if (rc < 0)
		pr_err("Error during bus state retrieval, rc=%d\n", rc);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_get_bus_state);

/* 
                                                          
                                        
                                         
  
                                                      
 */
int bif_ctrl_set_bus_state(struct bif_ctrl *ctrl, enum bif_bus_state state)
{
	int rc;

	if (IS_ERR_OR_NULL(ctrl)) {
		pr_err("Invalid controller handle.\n");
		return -ENODEV;
	}

	bif_ctrl_lock(ctrl);

	rc = ctrl->bdev->desc->ops->set_bus_state(ctrl->bdev, state);
	if (rc < 0)
		pr_err("Error during bus state configuration, rc=%d\n", rc);

	/*
                                                                        
                        
  */
	if (state == BIF_BUS_STATE_MASTER_DISABLED
	    || state == BIF_BUS_STATE_POWER_DOWN
	    || state == BIF_BUS_STATE_STANDBY)
		ctrl->bdev->selected_sdev = NULL;

	bif_ctrl_unlock(ctrl);

	return rc;
}
EXPORT_SYMBOL(bif_ctrl_set_bus_state);

/*
                                                                            
                                                    
 */
static int bif_initialize_protocol_function(struct bif_slave_dev *sdev,
		struct bif_ddb_l2_data *func)
{
	int rc = 0;
	u8 buf[4];

	/*                                          */
	if (func->function_type != BIF_FUNC_PROTOCOL)
		return 0;

	if (sdev->protocol_function) {
		pr_err("Duplicate protocol function found for BIF slave; DEV_ADR=0x%02X\n",
			sdev->slave_addr);
		return -EPERM;
	}

	sdev->protocol_function = kzalloc(sizeof(struct bif_protocol_function),
						GFP_KERNEL);
	if (!sdev->protocol_function) {
		pr_err("out of memory\n");
		return -ENOMEM;
	}

	rc = _bif_slave_read(sdev, func->function_pointer, buf, 4);
	if (rc) {
		pr_err("Protocol function data read failed, rc=%d\n", rc);
		return rc;
	}

	sdev->protocol_function->protocol_pointer  = buf[0] << 8 | buf[1];
	sdev->protocol_function->device_id_pointer = buf[2] << 8 | buf[3];
	sdev->protocol_function->l2_entry = func;

	rc = _bif_slave_read(sdev, sdev->protocol_function->device_id_pointer,
		sdev->protocol_function->device_id, BIF_DEVICE_ID_BYTE_LENGTH);
	if (rc) {
		pr_err("Device ID read failed, rc=%d\n", rc);
		return rc;
	}

	/*                                                       */
	if (sdev->unique_id_bits_known == 0) {
		sdev->unique_id_bits_known = BIF_UNIQUE_ID_BIT_LENGTH;
		/*                                                  */
		sdev->unique_id[0] = sdev->l1_data.manufacturer_id >> 8;
		sdev->unique_id[1] = sdev->l1_data.manufacturer_id;
		memcpy(&sdev->unique_id[2],
			sdev->protocol_function->device_id,
			BIF_DEVICE_ID_BYTE_LENGTH);
	}

	return rc;
}

/*
                                                                            
                                                              
 */
static int bif_initialize_slave_control_function(struct bif_slave_dev *sdev,
		struct bif_ddb_l2_data *func)
{
	int rc = 0;
	int i;
	u8 buf[3];

	/*                                               */
	if (func->function_type != BIF_FUNC_SLAVE_CONTROL)
		return 0;

	if (sdev->slave_ctrl_function) {
		pr_err("Duplicate slave control function found for BIF slave; DEV_ADR=0x%02X\n",
			sdev->slave_addr);
		return -EPERM;
	}

	sdev->slave_ctrl_function
		= kzalloc(sizeof(struct bif_protocol_function), GFP_KERNEL);
	if (!sdev->slave_ctrl_function) {
		pr_err("out of memory\n");
		return -ENOMEM;
	}

	rc = _bif_slave_read(sdev, func->function_pointer, buf, 3);
	if (rc) {
		pr_err("Slave control function data read failed, rc=%d\n", rc);
		return rc;
	}

	sdev->slave_ctrl_function->slave_ctrl_pointer = buf[0] << 8 | buf[1];
	sdev->slave_ctrl_function->task_count
		= buf[2] * SLAVE_CTRL_TASKS_PER_SET;
	sdev->slave_ctrl_function->l2_entry = func;

	if (sdev->slave_ctrl_function->task_count > 0) {
		sdev->slave_ctrl_function->irq_notifier_list =
			kzalloc(sizeof(struct blocking_notifier_head)
			    * sdev->slave_ctrl_function->task_count,
			    GFP_KERNEL);
		if (!sdev->slave_ctrl_function->irq_notifier_list) {
			pr_err("out of memory\n");
			kfree(sdev->slave_ctrl_function);
			return -ENOMEM;
		}

		for (i = 0; i < sdev->slave_ctrl_function->task_count; i++) {
			BLOCKING_INIT_NOTIFIER_HEAD(
			    &sdev->slave_ctrl_function->irq_notifier_list[i]);
		}
	}

	return rc;
}

/* 
                                                                            
                                        
                                           
  
                                                                        
                                                                          
                        
 */
u16 bif_crc_ccitt(const u8 *buffer, unsigned int len)
{
	u16 crc = 0xFFFF;

	while (len--) {
		crc = crc_ccitt_byte(crc, bitrev8(*buffer));
		buffer++;
	}
	return bitrev16(crc);
}
EXPORT_SYMBOL(bif_crc_ccitt);

static u16 bif_object_crc_ccitt(const struct bif_object *object)
{
	u16 crc = 0xFFFF;
	int i;

	crc = crc_ccitt_byte(crc, bitrev8(object->type));
	crc = crc_ccitt_byte(crc, bitrev8(object->version));
	crc = crc_ccitt_byte(crc, bitrev8(object->manufacturer_id >> 8));
	crc = crc_ccitt_byte(crc, bitrev8(object->manufacturer_id));
	crc = crc_ccitt_byte(crc, bitrev8(object->length >> 8));
	crc = crc_ccitt_byte(crc, bitrev8(object->length));

	for (i = 0; i < object->length - 8; i++)
		crc = crc_ccitt_byte(crc, bitrev8(object->data[i]));

	return bitrev16(crc);
}

/*
                                                                        
                                                                    
 */
static int bif_initialize_nvm_function(struct bif_slave_dev *sdev,
		struct bif_ddb_l2_data *func)
{
	int rc = 0;
	int data_len;
	u8 buf[8], object_type;
	struct bif_object *object;
	struct bif_object *temp;
	u16 addr;
	u16 crc;

	/*                                      */
	if (func->function_type != BIF_FUNC_NVM)
		return 0;

	if (sdev->nvm_function) {
		pr_err("Duplicate NVM function found for BIF slave; DEV_ADR=0x%02X\n",
			sdev->slave_addr);
		return -EPERM;
	}

	sdev->nvm_function
		= kzalloc(sizeof(*sdev->nvm_function), GFP_KERNEL);
	if (!sdev->nvm_function) {
		pr_err("out of memory\n");
		return -ENOMEM;
	}

	rc = _bif_slave_read(sdev, func->function_pointer, buf, 8);
	if (rc) {
		pr_err("NVM function data read failed, rc=%d\n", rc);
		return rc;
	}

	sdev->nvm_function->nvm_pointer		= buf[0] << 8 | buf[1];
	sdev->nvm_function->slave_control_channel	= buf[2];
	sdev->nvm_function->write_buffer_size		= buf[3];
	sdev->nvm_function->nvm_base_address		= buf[4] << 8 | buf[5];
	sdev->nvm_function->nvm_size			= buf[6] << 8 | buf[7];

	INIT_LIST_HEAD(&sdev->nvm_function->object_list);

	/*                  */
	addr = sdev->nvm_function->nvm_base_address;
	rc = _bif_slave_read(sdev, addr, &object_type, 1);
	if (rc) {
		pr_err("Slave memory read failed, rc=%d\n", rc);
		return rc;
	}

	/*                                                                */
	while (object_type != 0x00) {
		object = kzalloc(sizeof(*object), GFP_KERNEL);
		if (!object) {
			pr_err("out of memory\n");
			rc = -ENOMEM;
			goto free_data;
		}
		list_add_tail(&object->list, &sdev->nvm_function->object_list);

		rc = _bif_slave_read(sdev, addr + 1, buf + 1, 5);
		if (rc) {
			pr_err("Slave memory read of object header failed; addr=0x%04X, len=%d, rc=%d\n",
				addr + 1, 5, rc);
			goto free_data;
		}

		object->addr		= addr;
		object->type		= object_type;
		object->version		= buf[1];
		object->manufacturer_id	= buf[2] << 8 | buf[3];
		object->length		= buf[4] << 8 | buf[5];

		if ((object->addr + object->length)
		    > (sdev->nvm_function->nvm_base_address
				+ sdev->nvm_function->nvm_size)) {
			pr_warn("warning: BIF slave object is not formatted correctly; NVM base=0x%04X, NVM len=%d, object addr=0x%04X, object len=%d\n",
				sdev->nvm_function->nvm_base_address,
				sdev->nvm_function->nvm_size,
				object->addr,
				object->length);
			/*                                          */
			object->length = sdev->nvm_function->nvm_size
				+ sdev->nvm_function->nvm_base_address
				- object->addr;
		}

		/*                                       */
		data_len = object->length - 8;
		object->data = kmalloc(data_len, GFP_KERNEL);
		if (!object->data) {
			pr_err("out of memory\n");
			rc = -ENOMEM;
			goto free_data;
		}

		rc = _bif_slave_read(sdev, addr + 6, object->data, data_len);
		if (rc) {
			pr_err("Slave memory read of object data failed; addr=0x%04X, len=%d, rc=%d\n",
				addr + 6, data_len, rc);
			goto free_data;
		}

		rc = _bif_slave_read(sdev, addr + 6 + data_len, buf, 3);
		if (rc) {
			pr_err("Slave memory read of object CRC failed; addr=0x%04X, len=%d, rc=%d\n",
				addr + 6 + data_len, 3, rc);
			goto free_data;
		}

		object->crc = buf[0] << 8 | buf[1];
		object_type = buf[2];
		sdev->nvm_function->object_count++;

		crc = bif_object_crc_ccitt(object);
		if (crc != object->crc)
			pr_info("BIF object at addr=0x%04X has invalid CRC; crc calc=0x%04X, crc exp=0x%04X\n",
				object->addr, crc, object->crc);

		addr += object->length;
	}

	return rc;

free_data:
	list_for_each_entry_safe(object, temp,
				&sdev->nvm_function->object_list, list) {
		list_del(&object->list);
		kfree(object->data);
		kfree(object);
	}
	kfree(sdev->nvm_function);
	sdev->nvm_function = NULL;
	return rc;
}

static int bif_parse_slave_data(struct bif_slave_dev *sdev)
{
	int rc = 0;
	u8 buf[10];
	u8 *func_buf;
	struct bif_ddb_l2_data *func;
	int function_count, i;

	rc = _bif_slave_read(sdev, BIF_DDB_L1_BASE_ADDR, buf, 10);
	if (rc) {
		pr_err("DDB L1 data read failed, rc=%d\n", rc);
		return rc;
	}

	sdev->l1_data.revision		= buf[0];
	sdev->l1_data.level		= buf[1];
	sdev->l1_data.device_class	= buf[2] << 8 | buf[3];
	sdev->l1_data.manufacturer_id	= buf[4] << 8 | buf[5];
	sdev->l1_data.product_id	= buf[6] << 8 | buf[7];
	sdev->l1_data.length		= buf[8] << 8 | buf[9];

	function_count = sdev->l1_data.length / 4;
	if (sdev->l1_data.length % 4) {
		pr_err("Function directory length=%d is invalid\n",
				sdev->l1_data.length);
		return -EPROTO;
	}

	/*                              */
	if (function_count == 0)
		return 0;

	func_buf = kmalloc(sdev->l1_data.length, GFP_KERNEL);
	if (!func_buf) {
		pr_err("out of memory\n");
		return -ENOMEM;
	}

	sdev->function_directory = kzalloc(
		function_count * sizeof(struct bif_ddb_l2_data), GFP_KERNEL);
	if (!sdev->function_directory) {
		pr_err("out of memory\n");
		return -ENOMEM;
	}

	rc = _bif_slave_read(sdev, BIF_DDB_L2_BASE_ADDR, func_buf,
				sdev->l1_data.length);
	if (rc) {
		pr_err("DDB L2 data read failed, rc=%d\n", rc);
		return rc;
	}

	for (i = 0; i < function_count; i++) {
		func = &sdev->function_directory[i];
		func->function_type	= func_buf[i * 4];
		func->function_version	= func_buf[i * 4 + 1];
		func->function_pointer	= func_buf[i * 4 + 2] << 8
					  | func_buf[i * 4 + 3];
		rc = bif_initialize_protocol_function(sdev, func);
		if (rc)
			goto done;
		rc = bif_initialize_slave_control_function(sdev, func);
		if (rc)
			goto done;
		rc = bif_initialize_nvm_function(sdev, func);
		if (rc)
			goto done;
	}
done:
	kfree(func_buf);
	return rc;
}

static int bif_add_secondary_slaves(struct bif_slave_dev *primary_slave)
{
	int rc = 0;
	int data_len, i;
	u16 crc;
	struct bif_slave_dev *sdev;
	struct bif_object *object;

	list_for_each_entry(object, &primary_slave->nvm_function->object_list,
				list) {
		if (object->type != BIF_OBJ_SEC_SLAVE)
			continue;

		data_len = object->length - 8;
		if (data_len % BIF_UNIQUE_ID_BYTE_LENGTH) {
			pr_info("Invalid secondary slave object found, addr=0x%04X, data len=%d\n",
				object->addr, data_len);
			continue;
		}

		crc = bif_object_crc_ccitt(object);
		if (crc != object->crc) {
			pr_info("BIF object at addr=0x%04X has invalid CRC; crc calc=0x%04X, crc exp=0x%04X\n",
				object->addr, crc, object->crc);
			continue;
		}

		for (i = 0; i < data_len / BIF_UNIQUE_ID_BYTE_LENGTH; i++) {
			sdev = bif_add_slave(primary_slave->bdev);
			if (IS_ERR(sdev)) {
				rc = PTR_ERR(sdev);
				pr_err("bif_add_slave failed, rc=%d\n", rc);
				return rc;
			}
			memcpy(sdev->unique_id,
				&object->data[i * BIF_UNIQUE_ID_BYTE_LENGTH],
				BIF_UNIQUE_ID_BYTE_LENGTH);
			sdev->unique_id_bits_known = BIF_UNIQUE_ID_BIT_LENGTH;

			rc = bif_select_slave(sdev);
			if (rc) {
				pr_err("Could not select slave, rc=%d\n", rc);
				goto free_slave;
			}

			rc = bif_is_slave_selected(sdev->bdev);
			if (rc < 0) {
				pr_err("Transaction failed, rc=%d\n", rc);
				goto free_slave;
			} else if (rc == 1) {
				sdev->present = true;
				sdev->bdev->selected_sdev = sdev;
			} else {
				sdev->present = false;
				sdev->bdev->selected_sdev = NULL;
			}
		}
	}

	return rc;

free_slave:
	bif_remove_slave(sdev);
	return rc;
}

/*
                                                                               
                                
 */
static int bif_perform_uid_search(struct bif_ctrl_dev *bdev)
{
	struct bif_slave_dev *sdev;
	struct bif_slave_dev *new_slave;
	bool resp[2], resp_dilc;
	int i;
	int rc = 0;
	u8 cmd_probe[2] = {BIF_CMD_DIP0, BIF_CMD_DIP1};
	u8 cmd_enter[2] = {BIF_CMD_DIE0, BIF_CMD_DIE1};

	/*
                                                                     
          
  */
	list_for_each_entry(sdev, &bif_sdev_list, list) {
		/*                                    */
		if (sdev->unique_id_bits_known == BIF_UNIQUE_ID_BIT_LENGTH
		    || sdev->bdev != bdev)
			continue;

		/*                         */
		rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_BC,
							BIF_CMD_DISS);
		if (rc) {
			pr_err("bus_transaction failed, rc=%d\n", rc);
			return rc;
		}

		/*                                               */
		for (i = 0; i < sdev->unique_id_bits_known; i++) {
			rc = bdev->desc->ops->bus_transaction(bdev,
				BIF_TRANS_BC,
				cmd_enter[get_uid_bit(sdev->unique_id, i)]);
			if (rc) {
				pr_err("bus_transaction failed, rc=%d\n", rc);
				return rc;
			}
		}

		/*                                */
		for (i = sdev->unique_id_bits_known;
				i < BIF_UNIQUE_ID_BIT_LENGTH; i++) {
			rc = bdev->desc->ops->bus_transaction_query(bdev,
				BIF_TRANS_BC, cmd_probe[0], &resp[0]);
			if (rc) {
				pr_err("bus_transaction failed, rc=%d\n", rc);
				return rc;
			}

			rc = bdev->desc->ops->bus_transaction_query(bdev,
				BIF_TRANS_BC, cmd_probe[1], &resp[1]);
			if (rc) {
				pr_err("bus_transaction failed, rc=%d\n", rc);
				return rc;
			}

			if (resp[0] && resp[1]) {
				/*                                         */
				new_slave = bif_add_slave(bdev);
				if (IS_ERR(new_slave)) {
					rc = PTR_ERR(sdev);
					pr_err("bif_add_slave failed, rc=%d\n",
						rc);
					return rc;
				}
				memcpy(new_slave->unique_id, sdev->unique_id,
					BIF_UNIQUE_ID_BYTE_LENGTH);
				new_slave->bdev = sdev->bdev;

				set_uid_bit(sdev->unique_id, i, 0);
				sdev->unique_id_bits_known = i + 1;

				set_uid_bit(new_slave->unique_id, i, 1);
				new_slave->unique_id_bits_known = i + 1;
			} else if (resp[0]) {
				set_uid_bit(sdev->unique_id, i, 0);
				sdev->unique_id_bits_known = i + 1;
			} else if (resp[1]) {
				set_uid_bit(sdev->unique_id, i, 1);
				sdev->unique_id_bits_known = i + 1;
			} else {
				pr_debug("no bus query response received\n");
				rc = -ENXIO;
				return rc;
			}

			rc = bdev->desc->ops->bus_transaction(bdev,
				BIF_TRANS_BC, cmd_enter[resp[0] ? 0 : 1]);
			if (rc) {
				pr_err("bus_transaction failed, rc=%d\n", rc);
				return rc;
			}
		}

		rc = bdev->desc->ops->bus_transaction_query(bdev,
			BIF_TRANS_BC, BIF_CMD_DILC, &resp_dilc);
		if (rc) {
			pr_err("bus_transaction failed, rc=%d\n", rc);
			return rc;
		}

		if (resp_dilc) {
			sdev->present = true;
			sdev->bdev->selected_sdev = sdev;
			rc = bif_parse_slave_data(sdev);
		} else {
			pr_err("Slave failed to respond to DILC bus command; its UID is thus unverified.\n");
			sdev->unique_id_bits_known = 0;
			rc = -ENXIO;
			return rc;
		}
	}

	return rc;
}

/*
                                                                            
                      
 */
static int bif_remove_duplicate_slaves(struct bif_ctrl_dev *bdev)
{
	struct bif_slave_dev *sdev;
	struct bif_slave_dev *last_slave;
	struct bif_slave_dev *temp;

	list_for_each_entry_safe(last_slave, temp, &bif_sdev_list, list) {
		list_for_each_entry(sdev, &bif_sdev_list, list) {
			if (last_slave == sdev) {
				break;
			} else if (memcmp(last_slave->unique_id,
					sdev->unique_id,
					BIF_UNIQUE_ID_BYTE_LENGTH) == 0) {
				bif_remove_slave(last_slave);
				break;
			}
		}
	}

	return 0;
}

static int bif_add_all_slaves(struct bif_ctrl_dev *bdev)
{
	struct bif_slave_dev *sdev;
	int rc = 0;
	int i;
	bool has_slave = false, is_primary_slave = false;

	mutex_lock(&bif_sdev_list_mutex);
	mutex_lock(&bdev->mutex);

	list_for_each_entry(sdev, &bif_sdev_list, list) {
		if (sdev->bdev == bdev) {
			has_slave = true;
			break;
		}
	}

	if (!has_slave) {
		/*                                                            */
		sdev = bif_add_slave(bdev);
		if (IS_ERR(sdev)) {
			rc = PTR_ERR(sdev);
			pr_err("bif_add_slave failed, rc=%d\n", rc);
			goto out;
		}

		for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
			/*                                                  */
			rc = bdev->desc->ops->bus_transaction(bdev,
				BIF_TRANS_SDA, BIF_PRIMARY_SLAVE_DEV_ADR);
			if (rc == 0)
				break;
		}
		if (rc) {
			pr_err("BIF bus_transaction failed, rc=%d\n", rc);
			goto out;
		}

		/*                               */
		rc = bif_is_slave_selected(bdev);
		if (rc < 0) {
			pr_err("BIF bus_transaction failed, rc=%d\n", rc);
			goto out;
		} else {
			is_primary_slave = rc;
		}
	}

	if (is_primary_slave) {
		pr_debug("Using primary slave at DEV_ADR==0x%02X\n",
			BIF_PRIMARY_SLAVE_DEV_ADR);
		sdev->bdev->selected_sdev = sdev;
		sdev->present = true;
		sdev->slave_addr = BIF_PRIMARY_SLAVE_DEV_ADR;
		rc = bif_parse_slave_data(sdev);
		if (rc) {
			pr_err("Failed to parse primary slave data, rc=%d\n",
				rc);
			goto out;
		}
		rc = bif_add_secondary_slaves(sdev);
		if (rc) {
			pr_err("Failed to add secondary slaves, rc=%d\n", rc);
			goto out;
		}
	} else {
		pr_debug("Falling back on full UID search.\n");
		for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
			rc = bif_perform_uid_search(bdev);
			if (rc == 0)
				break;
		}
		if (rc) {
			pr_debug("BIF UID search failed, rc=%d\n", rc);
			goto out;
		}
	}

	bif_remove_duplicate_slaves(bdev);

	mutex_unlock(&bdev->mutex);
	mutex_unlock(&bif_sdev_list_mutex);

	return rc;

out:
	mutex_unlock(&bdev->mutex);
	mutex_unlock(&bif_sdev_list_mutex);
	pr_debug("BIF slave search failed, rc=%d\n", rc);
	return rc;
}

static int bif_add_known_slave(struct bif_ctrl_dev *bdev, u8 slave_addr)
{
	struct bif_slave_dev *sdev;
	int rc = 0;
	int i;

	for (i = 0; i < BIF_TRANSACTION_RETRY_COUNT; i++) {
		/*                              */
		rc = bdev->desc->ops->bus_transaction(bdev, BIF_TRANS_SDA,
							slave_addr);
		if (rc == 0)
			break;
	}
	if (rc) {
		pr_err("BIF bus_transaction failed, rc=%d\n", rc);
		return rc;
	}

	/*                               */
	rc = bif_is_slave_selected(bdev);
	if (rc < 0) {
		pr_err("BIF bus_transaction failed, rc=%d\n", rc);
		return rc;
	}

	sdev = bif_add_slave(bdev);
	if (IS_ERR(sdev)) {
		rc = PTR_ERR(sdev);
		pr_err("bif_add_slave failed, rc=%d\n", rc);
		return rc;
	}

	sdev->bdev->selected_sdev = sdev;
	sdev->present = true;
	sdev->slave_addr = slave_addr;
	rc = bif_parse_slave_data(sdev);
	if (rc) {
		pr_err("Failed to parse slave data, addr=0x%02X, rc=%d\n",
			slave_addr, rc);
		return rc;
	}

	return rc;
}

static int bif_add_known_slaves_from_dt(struct bif_ctrl_dev *bdev,
					struct device_node *of_node)
{
	int len = 0;
	int rc, i;
	u32 addr;
	const __be32 *val;

	mutex_lock(&bif_sdev_list_mutex);
	mutex_lock(&bdev->mutex);

	val = of_get_property(of_node, "qcom,known-device-addresses", &len);
	len /= sizeof(u32);
	if (val && len == 0) {
		pr_err("qcom,known-device-addresses property is invalid\n");
		rc = -EINVAL;
		goto out;
	}

	for (i = 0; i < len; i++) {
		addr = be32_to_cpup(val++);
		if (addr == 0x00 || addr > 0xFF) {
			rc = -EINVAL;
			pr_err("qcom,known-device-addresses property contains invalid address=0x%X\n",
				addr);
			goto out;
		}
		rc = bif_add_known_slave(bdev, addr);
		if (rc) {
			pr_err("bif_add_known_slave() failed, rc=%d\n", rc);
			goto out;
		}
	}

out:
	if (len > 0)
		bif_remove_duplicate_slaves(bdev);

	mutex_unlock(&bdev->mutex);
	mutex_unlock(&bif_sdev_list_mutex);

	return rc;
}

/*
                                                                         
                                 
 */
static int bif_assign_slave_dev_addr(struct bif_slave_dev *sdev, u8 dev_addr)
{
	int rc;
	u16 addr;

	if (!sdev->protocol_function) {
		pr_err("Protocol function not present; cannot set device address.\n");
		return -ENODEV;
	}

	addr = PROTOCOL_FUNC_DEV_ADR_ADDR(
			sdev->protocol_function->protocol_pointer);

	rc = _bif_slave_write(sdev, addr, &dev_addr, 1);
	if (rc)
		pr_err("Failed to set slave device address.\n");
	else
		sdev->slave_addr = dev_addr;

	return rc;
}

/*                                                                      */
static int bif_assign_all_slaves_dev_addr(struct bif_ctrl_dev *bdev)
{
	struct bif_slave_dev *sdev;
	struct bif_slave_dev *sibling;
	bool duplicate;
	int rc = 0;
	u8 dev_addr, first_dev_addr;

	mutex_lock(&bif_sdev_list_mutex);
	mutex_lock(&bdev->mutex);

	first_dev_addr = next_dev_addr;
	/*
                                                                     
          
  */
	list_for_each_entry(sdev, &bif_sdev_list, list) {
		/*
                                                                
                                     
   */
		if (sdev->unique_id_bits_known != BIF_UNIQUE_ID_BIT_LENGTH
		    || sdev->slave_addr != 0x00 || !sdev->present)
			continue;

		do {
			dev_addr = next_dev_addr;
			duplicate = false;
			list_for_each_entry(sibling, &bif_sdev_list, list) {
				if (sibling->slave_addr == dev_addr) {
					duplicate = true;
					break;
				}
			}

			next_dev_addr = dev_addr + 1;
		} while (duplicate && (next_dev_addr != first_dev_addr));

		if (next_dev_addr == first_dev_addr) {
			pr_err("No more BIF slave device addresses available.\n");
			rc = -ENODEV;
			goto out;
		}

		rc =  bif_assign_slave_dev_addr(sdev, dev_addr);
		if (rc) {
			pr_err("Failed to set slave address.\n");
			goto out;
		}
	}

	mutex_unlock(&bdev->mutex);
	mutex_unlock(&bif_sdev_list_mutex);

	return rc;

out:
	mutex_unlock(&bdev->mutex);
	mutex_unlock(&bif_sdev_list_mutex);
	pr_err("BIF slave device address setting failed, rc=%d\n", rc);
	return rc;
}

/* 
                                                                  
                                       
 */
void *bdev_get_drvdata(struct bif_ctrl_dev *bdev)
{
	return bdev->driver_data;
}
EXPORT_SYMBOL(bdev_get_drvdata);

static const char * const battery_label[] = {
	"unknown",
	"none",
	"special 1",
	"special 2",
	"special 3",
	"low cost",
	"smart",
};

static const char *bif_get_battery_pack_type(int rid_ohm)
{
	const char *label = battery_label[0];

	if (rid_ohm > BIF_BATT_RID_SMART_MAX)
		label = battery_label[1];
	else if (rid_ohm >= BIF_BATT_RID_SMART_MIN)
		label = battery_label[6];
	else if (rid_ohm >= BIF_BATT_RID_LOW_COST_MIN
			&& rid_ohm <= BIF_BATT_RID_LOW_COST_MAX)
		label = battery_label[5];
	else if (rid_ohm >= BIF_BATT_RID_SPECIAL3_MIN
			&& rid_ohm <= BIF_BATT_RID_SPECIAL3_MAX)
		label = battery_label[4];
	else if (rid_ohm >= BIF_BATT_RID_SPECIAL2_MIN
			&& rid_ohm <= BIF_BATT_RID_SPECIAL2_MAX)
		label = battery_label[3];
	else if (rid_ohm >= BIF_BATT_RID_SPECIAL1_MIN
			&& rid_ohm <= BIF_BATT_RID_SPECIAL1_MAX)
		label = battery_label[2];

	return label;
}

/* 
                                                                         
                                                   
                                              
                                                                         
                                                                  
  
                                                                             
                                                    
 */
struct bif_ctrl_dev *bif_ctrl_register(struct bif_ctrl_desc *bif_desc,
	struct device *dev, void *driver_data, struct device_node *of_node)
{
	struct bif_ctrl_dev *bdev = ERR_PTR(-EINVAL);
	struct bif_slave_dev *sdev;
	bool battery_present = false;
	int rc, rid_ohm;

	if (!bif_desc) {
		pr_err("Invalid bif_desc specified\n");
		return bdev;
	} else if (!bif_desc->name) {
		pr_err("BIF name missing\n");
		return bdev;
	} else if (!bif_desc->ops) {
		pr_err("BIF operations missing\n");
		return bdev;
	} else if (!bif_desc->ops->bus_transaction
			|| !bif_desc->ops->bus_transaction_query
			|| !bif_desc->ops->bus_transaction_read
			|| !bif_desc->ops->get_bus_state
			|| !bif_desc->ops->set_bus_state) {
		pr_err("BIF operation callback function(s) missing\n");
		return bdev;
	}

	bdev = kzalloc(sizeof(struct bif_ctrl_dev), GFP_KERNEL);
	if (bdev == NULL) {
		pr_err("Memory allocation failed for bif_ctrl_dev\n");
		return ERR_PTR(-ENOMEM);
	}

	mutex_init(&bdev->mutex);
	INIT_LIST_HEAD(&bdev->list);
	INIT_DELAYED_WORK(&bdev->enter_irq_mode_work, bif_enter_irq_mode_work);
	bdev->desc			= bif_desc;
	bdev->ctrl_dev			= dev;
	bdev->driver_data		= driver_data;
	bdev->irq_mode_delay_jiffies	= 2;

	mutex_lock(&bif_ctrl_list_mutex);
	list_add_tail(&bdev->list, &bif_ctrl_list);
	mutex_unlock(&bif_ctrl_list_mutex);

	rc = bif_add_all_slaves(bdev);
	if (rc)
		pr_debug("Search for all slaves failed, rc=%d\n", rc);
	rc = bif_add_known_slaves_from_dt(bdev, of_node);
	if (rc)
		pr_err("Adding slaves based on device tree addressed failed, rc=%d.\n",
			rc);
	rc = bif_assign_all_slaves_dev_addr(bdev);
	if (rc)
		pr_err("Failed to set slave device address, rc=%d\n", rc);

	bif_print_slaves();

	if (bdev->desc->ops->get_battery_presence) {
		rc = bdev->desc->ops->get_battery_presence(bdev);
		if (rc < 0) {
			pr_err("Could not determine battery presence, rc=%d\n",
				rc);
		} else {
			battery_present = rc;
			pr_info("Battery pack present = %c\n", rc ? 'Y' : 'N');
		}
	}

	if (bdev->desc->ops->get_battery_rid) {
		rid_ohm = bdev->desc->ops->get_battery_rid(bdev);
		if (rid_ohm >= 0)
			pr_info("Battery pack type = %s (Rid=%d ohm)\n",
				bif_get_battery_pack_type(rid_ohm), rid_ohm);
		else
			pr_err("Could not read Rid, rc=%d\n", rid_ohm);
	}

	list_for_each_entry(sdev, &bif_sdev_list, list) {
		if (sdev->present) {
			battery_present = true;
			break;
		}
	}

	BLOCKING_INIT_NOTIFIER_HEAD(&bdev->bus_change_notifier);

	if (battery_present) {
		bdev->battery_present = true;
		rc = blocking_notifier_call_chain(&bdev->bus_change_notifier,
			BIF_BUS_EVENT_BATTERY_INSERTED, bdev);
		if (rc)
			pr_err("Call chain noification failed, rc=%d\n", rc);
	}

	return bdev;
}
EXPORT_SYMBOL(bif_ctrl_register);

/* 
                                                       
                                       
 */
void bif_ctrl_unregister(struct bif_ctrl_dev *bdev)
{
	if (bdev) {
		mutex_lock(&bif_ctrl_list_mutex);
		list_del(&bdev->list);
		mutex_unlock(&bif_ctrl_list_mutex);
	}
}
EXPORT_SYMBOL(bif_ctrl_unregister);
