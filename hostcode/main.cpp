#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "gl_debug.hpp"
#include "arduinoserialio/arduinoSerial.hpp"
#include <thread>
#include <vector>

#define DEBUG true // Enables opengl debug info callback
#define RESPONSE_MAX_SIZE 256 // Guesstimated buffer size for receiving response from arm to commands
#define BAUD_RATE B115200
#define ADJUSTMENT_START 10.0 // Degree adjustment per step
#define SPEED_START 90 // Degrees per second speed


/*
    Reads all the current joint positions from the arm.
    Currently only included as way to get potential debug info.
*/
std::vector<double> getJointPositions(arduinoSerial& Serial){
    Serial.flush();
    Serial.print("READ\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    char response[RESPONSE_MAX_SIZE];
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "getJointPositions(): Read response: " << response << std::endl;
    double numbers[6] = {157.5,157.5,187.5,57.5,157.5,90}; // Fallback to home position if sscanf fails
    if(std::sscanf(response, "[%lf,%lf,%lf,%lf,%lf,%lf]", &numbers[0], &numbers[1], &numbers[2], &numbers[3], &numbers[4], &numbers[5]) != 6){
        std::cout << "ERR: Failed to parse response\n";
    }else{
        std::cout << "getJointPositions(): Parsed response: " << numbers[0] << ", " << numbers[1] << ", " << numbers[2] << ", " << numbers[3] << ", " << numbers[4] << ", " << numbers[5] << "\n";
    }
    std::vector<double> v(numbers, numbers + sizeof(numbers) / sizeof(numbers[0]));
    return v;
}


/*
    Currently unused, but useful for setting multiple joint positions in one go
    Keeping here for now just in case it becomes useful
*/
void setJointPositions(arduinoSerial& Serial, std::vector<double> positions){
    std::string cmd = "SETX";
    for(double& p : positions){
        cmd += " " + std::to_string(p);
    }
    Serial.print(cmd);
}


/*
    Moves one of the joints (specified by idx, which ranges from 1-6) by adj degrees
*/
void adjustJointPos(arduinoSerial& Serial, int idx, double adj){
    if(idx > 6 || idx < 1){
        throw std::runtime_error("ERR: adjustJointPos() passed invalid idx");
    }
    Serial.print("MOVE " + std::to_string(idx) + " " + std::to_string(adj) + "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    char response[RESPONSE_MAX_SIZE] = {0};
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "adjustJointPos(): Read response: " << response << std::endl;
}


/*
    Sets the speed of the movement of the robot arm.
    Speed is in degrees/second
*/
void setSpeed(arduinoSerial& Serial, int speed){
    Serial.print("SPEED " + std::to_string(speed) + "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    char response[RESPONSE_MAX_SIZE] = {0};
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "setSpeed(): Read response: " << response << std::endl;
}


/*
    Returns the arm to a "starting" position of sorts.
    {157.5, 157.5, 187.5, 57.5, 157.5, 90}
*/
void homeArm(arduinoSerial& Serial){
    Serial.print("HOME\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    char response[RESPONSE_MAX_SIZE] = {0};
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "homeArm(): Read response: " << response << std::endl;
}


int main(){
    // Initialise serial communication
    arduinoSerial Serial; // Provides very arduino-like functions for interacting with a serial device
    Serial.openPort("/dev/ttyACM0"); // Default file for an arduino uno
    Serial.begin(BAUD_RATE);

    // Begin GLFW + OpenGL boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Use OpenGL 3.3 core profile:
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(512, 512, "Arm controls", nullptr, nullptr);
    if(!window){
        throw std::runtime_error("ERR: Failed to create window");
    }
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height){
        glViewport(0, 0, width, height); // Very basic size callback
    });
    glfwMakeContextCurrent(window);
    if(!gladLoaderLoadGL()){ // Load/init GLAD
        throw std::runtime_error("ERR: Failed to load GLAD");
    }
    if(DEBUG){ // Handy debugging error messages from OpenGL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(debug::messageCallback, nullptr);
    }
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // End GLFW + OpenGL boilerplate

    setSpeed(Serial, SPEED_START);
    double adjustment = ADJUSTMENT_START;
    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents(); // Check for events (keyboard, mouse, etc)

        /*
            Home the arm
        */
        if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
            homeArm(Serial);
        }

        /*
            Print the current joint positions (for debugging, etc)
        */
        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
            std::vector<double> jointPositions = getJointPositions(Serial);
            for(double& j : jointPositions){
                std::cout << j << ", ";
            }
            std::cout << "\n";
        }

        /*
            Per-joint control, a bit difficult to use but easy to implement
        */
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
            adjustJointPos(Serial, 1, adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
            adjustJointPos(Serial, 1, -adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
            adjustJointPos(Serial, 2, adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
            adjustJointPos(Serial, 2, -adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            adjustJointPos(Serial, 3, adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            adjustJointPos(Serial, 3, -adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            adjustJointPos(Serial, 4, adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            adjustJointPos(Serial, 4, -adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            adjustJointPos(Serial, 5, adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            adjustJointPos(Serial, 5, -adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
            adjustJointPos(Serial, 6, adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
            adjustJointPos(Serial, 6, -adjustment);
        }

        glfwSwapBuffers(window); // Render the current frame
    }

    // Cleanup
    glfwTerminate();
    return 0;
}