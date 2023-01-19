/**
 * @file   gpio_test.c
 * @author Rajesh
 * @date   17 Jan 2023
 * @brief  A kernel module for controlling a GPIO LED/button pair in Variscite iMX6UL DART Eval Board
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>                 // Required for the GPIO functions
#include <linux/interrupt.h>            // Required for the IRQ code

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rajesh");
MODULE_DESCRIPTION("A Button/LED test driver for the iMX6 UL DART");
MODULE_VERSION("1.0");

static unsigned int gpioLED = 120;       ///< hard coding the LED gpio for this example to J13_2 of VAR-DART-IMX6UL
static unsigned int gpioButton = 0;  ///< hard coding the button gpio for this example to SW4 of VAR-DART-IMX6UL (GPIO1[0])
static unsigned int irqNumber;          ///< Used to share the IRQ number within this file
static unsigned int numberPresses = 0;  ///< For information, store the number of button presses
static bool	    ledOn = 0;          ///< Is the LED on or off? Used to invert its state (off by default)

/// Function prototype for the custom IRQ handler function 

static irq_handler_t  ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

// initialization function

 
static int __init ebbgpio_init(void){
   int result = 0;
   printk(KERN_INFO "GPIO_TEST: Initializing the GPIO_TEST LKM\n");
   // Is the GPIO a valid GPIO number)
   if (!gpio_is_valid(gpioLED)){
      printk(KERN_INFO "GPIO_TEST: invalid LED GPIO\n");
      return -ENODEV;
   }
   
   ledOn = true;
   gpio_request(gpioLED, "sysfs");          
   gpio_direction_output(gpioLED, ledOn);  
// gpio_set_value(gpioLED, ledOn);          
   gpio_export(gpioLED, false);             
			                    
   gpio_request(gpioButton, "sysfs");      
   gpio_direction_input(gpioButton);       
   gpio_set_debounce(gpioButton, 200);      // Debounce the button with a delay of 200ms
   gpio_export(gpioButton, false);         
			                   

   printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));

  
   irqNumber = gpio_to_irq(gpioButton);
   printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);

 
   result = request_irq(irqNumber,            
                        (irq_handler_t) ebbgpio_irq_handler, 
                        IRQF_TRIGGER_RISING,   
                        "ebb_gpio_handler",    
                        NULL);                

   printk(KERN_INFO "GPIO_TEST: The interrupt request result is: %d\n", result);
   return result;
}


static void __exit ebbgpio_exit(void){
   printk(KERN_INFO "GPIO_TEST: The button state is currently: %d\n", gpio_get_value(gpioButton));
   printk(KERN_INFO "GPIO_TEST: The button was pressed %d times\n", numberPresses);
   gpio_set_value(gpioLED, 0);             
   gpio_unexport(gpioLED);                 
   free_irq(irqNumber, NULL);               
   gpio_unexport(gpioButton);               
   gpio_free(gpioLED);                      
   gpio_free(gpioButton);                  
   printk(KERN_INFO "GPIO_TEST: Goodbye from the LKM!\n");
}


static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   ledOn = !ledOn;                          // Invert the LED state on each button press
   gpio_set_value(gpioLED, ledOn);          
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d)\n", gpio_get_value(gpioButton));
   numberPresses++;                         
   return (irq_handler_t) IRQ_HANDLED;     
}


module_init(ebbgpio_init);
module_exit(ebbgpio_exit);

