#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <geometry_msgs/msg/twist.h>

#include <rclc/rclc.h>
#include <rclc/executor.h>

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"
#endif

#define ADC1_CHANNEL_4    ADC1_CHANNEL_4	//ADC1 channel 4 is GPIO32 (ESP32)
#define ADC1_CHANNEL_5    ADC1_CHANNEL_5	//ADC1 channel 5 is GPIO33 (ESP32)

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);vTaskDelete(NULL);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

static const adc_bits_width_t width = ADC_WIDTH_BIT_9;

//initialization
rcl_publisher_t publisher;

esp_err_t r1, r2;

// pin definition
gpio_num_t adc_gpio_num1, adc_gpio_num2;


//value map function
float mapValue(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
	RCLC_UNUSED(last_call_time);
	if (timer != NULL) {
		//initialization of twist msg
		geometry_msgs__msg__Twist twist;
		geometry_msgs__msg__Twist__init(&twist);

		// read values
		int x_axis_read = adc1_get_raw(ADC1_CHANNEL_4) *1.15 ;	// 1.15 is obtained after hardware testing 
		int y_axis_read = adc1_get_raw(ADC1_CHANNEL_5) *1.15 ;	// 1.15 is obtained after hardware testing 
		
		// assign message to publisher		  		
		// map values from 0 - 511 (ADC input) to (-3) - 3 (for turtlesim speed)
  		twist.linear.x =  mapValue(x_axis_read, 0, 511, -3,  3);  
		twist.angular.z = mapValue(y_axis_read, 0, 511,  3, -3);  
		
		// print values in serial monitor 
		//printf("analog read x= %d \t y= %d \n", x_axis_read , y_axis_read );
		//printf("twist out x= %f \t z= %f \n", twist.linear.x , twist.angular.z );
		
		RCSOFTCHECK(rcl_publish(&publisher, &twist, NULL));		
	}	
}


void appMain(void * arg)
{
	rcl_allocator_t allocator = rcl_get_default_allocator();
	rclc_support_t support;

	r1 = adc1_pad_get_io_num( ADC1_CHANNEL_4, &adc_gpio_num1 );
    r2 = adc1_pad_get_io_num( ADC1_CHANNEL_5, &adc_gpio_num2 );

    assert( r1 == ESP_OK && r2 == ESP_OK);

    printf("ADC1 channel_4 %d @ GPIO %d, channel_5 %d @ GPIO %d.\n", ADC1_CHANNEL_4, adc_gpio_num1,
                ADC1_CHANNEL_5, adc_gpio_num2 );

	//ADC initialization
    printf("adc1_init...\n");    
    adc1_config_channel_atten( ADC1_CHANNEL_4, ADC_ATTEN_11db );
    adc1_config_channel_atten( ADC1_CHANNEL_5, ADC_ATTEN_11db );
    adc1_config_width(width);

	// create init_options
	RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

	// create node
	rcl_node_t node;
	RCCHECK(rclc_node_init_default(&node, "freertos_twist_pub", "", &support));

	// create publisher
	RCCHECK(rclc_publisher_init_default(
		&publisher,
		&node,
		ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
		"/turtle1/cmd_vel"));
	
	// create timer,
	rcl_timer_t timer;
	const unsigned int timer_timeout = 500;  //publish timeout
	RCCHECK(rclc_timer_init_default(
		&timer,
		&support,
		RCL_MS_TO_NS(timer_timeout),
		timer_callback));

	// create executor
	rclc_executor_t executor;
	RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
	RCCHECK(rclc_executor_add_timer(&executor, &timer));

	while(1){
		rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));		
		usleep(100000);
	}

	// free resources
	RCCHECK(rcl_publisher_fini(&publisher, &node))
	RCCHECK(rcl_node_fini(&node))

  	vTaskDelete(NULL);
}
