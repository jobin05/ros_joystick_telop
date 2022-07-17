
#include <ros/ros.h>
#include "nav_msgs/Odometry.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>

int read_event(int fd, struct js_event *event)
{
    ssize_t bytes;
    bytes = read(fd, event, sizeof(*event));
    if (bytes == sizeof(*event))
    {

        return 0;
    }

    return -1;
}   
struct axis_state
{
    short x, y;
};
size_t get_axis_state(struct js_event *event, struct axis_state axes[3])
{
    size_t axis = event->number / 2;

    if (axis < 3)
    {
        if (event->number % 2 == 0)
            axes[axis].x = event->value;
        else
            axes[axis].y = event->value;
    }

    return axis;
}
int fd_set_blocking(int fd, int blocking)
{
    /* Save the current flags */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return 0;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "joystick_telop_node");

    const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;
    float pre_linear_value = 0;

    struct timeval waitd = {10, 0}; // the max wait time for an event
    int sel;                        // holds return value for select(); int sel;

    if (argc > 1)
        device = argv[1];
    else
        device = "/dev/input/js0";

    js = open(device, O_RDONLY);

    if (js == -1)
    {
        ROS_INFO("Could not open joystick");
    }

    else
        ROS_INFO("Done");

    

    ros::NodeHandle nodehadle;

    ros::Publisher run = nodehadle.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

    ros::Rate loop_rate(50);
   
    while (ros::ok())
    {

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(js, &fds);
        sel = select(js + 1, &fds, 0, (fd_set *)0, &waitd);
        if (FD_ISSET(js, &fds))
        {
            // ROS_INFO("Something to read");
            FD_CLR(js, &fds);

            if (read_event(js, &event) == 0)
            {
                // ROS_INFO("USB Readed Event");
                switch (event.type)
                {
                case JS_EVENT_BUTTON:
                    ROS_INFO("Button %u %s\n", event.number, event.value ? "pressed" : "released");
                    break;
                case JS_EVENT_AXIS:
                    axis = get_axis_state(&event, axes);
                    if (axis < 3)
                    {
                        // ROS_INFO("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
                        //-2.84
                        geometry_msgs::Twist tw;

                        if (axis == 1)
                        {
                            float outputvalue = map(axes[axis].x, -32767, 32767, -.22, .22);
                            tw.linear.x = outputvalue;
                            pre_linear_value = outputvalue;
                            // long double outputvalue = 12.4;
                            ROS_INFO("X %f   %f", outputvalue, pre_linear_value);
                        }
                        else if (axis == 0)
                        {
                            float outputvalue = map(axes[axis].x, -32767, 32767, -2.84, 2.84);
                            tw.angular.z = outputvalue;
                            tw.linear.x = pre_linear_value;
                            // long double outputvalue = 12.4;
                            ROS_INFO("Y %f   %f", outputvalue, pre_linear_value);
                        }
                        run.publish(tw);
                    }

                    break;
                default:
                    break;
                }
            }
        }
        else
        {
        }

        ros::spinOnce();
        loop_rate.sleep();
        // ROS_INFO("Loop");
       
    }

   
    ros::shutdown();
    return 0;
}
