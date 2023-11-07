#include <alchemy/task.h>
#include <stdio.h>

void led_blink_high_prio(void *arg) 
{
	 int gpio_pin = 44; // GPIO number for physical pin P9_12 (GPIO_60)
    int led_state = 0;

    // Export the GPIO pin
    char gpio_export_command[64];
    snprintf(gpio_export_command, sizeof(gpio_export_command), "echo %d > /sys/class/gpio/export", gpio_pin);
    system(gpio_export_command);

    // Set the GPIO direction to out
    char gpio_direction_command[64];
    snprintf(gpio_direction_command, sizeof(gpio_direction_command), "echo out > /sys/class/gpio/gpio%d/direction", gpio_pin);
    system(gpio_direction_command);

    while (1) {
        // Toggle the LED state
        led_state = !led_state;
        // Set the GPIO pin value
        char gpio_value_command[64];
        snprintf(gpio_value_command, sizeof(gpio_value_command), "echo %d > /sys/class/gpio/gpio%d/value", led_state, gpio_pin);
        system(gpio_value_command);

        rt_task_sleep(1000000000); // Sleep for 1 second
    }

    // Unexport the GPIO pin (cleanup)
    char gpio_unexport_command[64];
    snprintf(gpio_unexport_command, sizeof(gpio_unexport_command), "echo %d > /sys/class/gpio/unexport", gpio_pin);
    system(gpio_unexport_command);
}

void led_blink_low_prio(void *arg)
{
 int gpio_pin = 46; // GPIO number for physical pin P9_12 (GPIO_60)
    int led_state = 0;

    // Export the GPIO pin
    char gpio_export_command[64];
    snprintf(gpio_export_command, sizeof(gpio_export_command), "echo %d > /sys/class/gpio/export", gpio_pin);
    system(gpio_export_command);

    // Set the GPIO direction to out
    char gpio_direction_command[64];
    snprintf(gpio_direction_command, sizeof(gpio_direction_command), "echo out > /sys/class/gpio/gpio%d/direction", gpio_pin);
    system(gpio_direction_command);

    while (1) {
        // Toggle the LED state
        led_state = !led_state;
        // Set the GPIO pin value
        char gpio_value_command[64];
        snprintf(gpio_value_command, sizeof(gpio_value_command), "echo %d > /sys/class/gpio/gpio%d/value", led_state, gpio_pin);
        system(gpio_value_command);

        rt_task_sleep(1000000000); // Sleep for 1 second
    }

    // Unexport the GPIO pin (cleanup)
    char gpio_unexport_command[64];
    snprintf(gpio_unexport_command, sizeof(gpio_unexport_command), "echo %d > /sys/class/gpio/unexport", gpio_pin);
    system(gpio_unexport_command);
}

int main(int argc, char *argv[])
{
	RT_TASK task_high_prio, task_low_prio;

    // Initialize Xenomai

    if (rt_task_create(&task_high_prio, "LED_Blink_High_Prio", 0, 90, T_JOINABLE) != 0) {
        printf("Error creating the high-priority task\n");
        return -1;
    }

    if (rt_task_start(&task_high_prio, &led_blink_high_prio, NULL) != 0) {
        printf("Error starting the high-priority task\n");
        return -1;
    }

    if (rt_task_create(&task_low_prio, "LED_Blink_Low_Prio", 0, 70, T_JOINABLE) != 0) {
        printf("Error creating the low-priority task\n");
        return -1;
    }

    if (rt_task_start(&task_low_prio, &led_blink_low_prio, NULL) != 0) {
        printf("Error starting the low-priority task\n");
        return -1;
    }

    // Wait for the tasks to finish (won't happen in this example)
    rt_task_join(&task_high_prio);
    rt_task_join(&task_low_prio);

    return 0;
}