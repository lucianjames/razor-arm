#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "gl_debug.hpp"
#include "arduinoserialio/arduinoSerial.hpp"
#include <thread>
#include <vector>
#include <time.h>

#define DEBUG true // Enables opengl debug info callback
#define RESPONSE_MAX_SIZE 256 // Guesstimated buffer size for receiving response from arm to commands
#define BAUD_RATE B115200
#define POS_ADJUSTMENT_START 15 // Degree adjustment per step
#define POS_ADJUSTMENT_ADJUSTMENT 1 // Degree adjustment per step adjustment
#define SPEED_ADJUSTMENT 10 // Degrees per second
#define SPEED_START 60 // Degrees per second speed
#define SYNC_TIMEOUT 100

int curr_speed = SPEED_START;
int wait_time = 1000;
/*
    Reads all the current joint positions from the arm.
    Currently only included as way to get potential debug info.
*/
std::vector<double> getJointPositions(arduinoSerial& Serial){
    Serial.flush();
    Serial.print("READ\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_TIMEOUT));
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
void setJointPositions(arduinoSerial& Serial, std::vector<double>& positions){
    std::string cmd = "SETX";
    for(double& p : positions){
        cmd += " " + std::to_string(p);
    }
    Serial.print(cmd + "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_TIMEOUT));
    char response[RESPONSE_MAX_SIZE] = {0};
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "setJointPositions(): Read response: " << response << std::endl;
}

/*
    Moves one of the joints (specified by idx, which ranges from 1-6) by adj degrees
*/
void adjustJointPos(arduinoSerial& Serial, int idx, double adj){
    if(idx > 6 || idx < 1){
        throw std::runtime_error("ERR: adjustJointPos() passed invalid idx");
    }
    Serial.print("MOVE " + std::to_string(idx) + " " + std::to_string(adj) + "\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_TIMEOUT));
    char response[RESPONSE_MAX_SIZE] = {0};
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "adjustJointPos(): Read response: " << response << std::endl;
}


/*
    Sets the speed of the movement of the robot arm.
    Speed is in degrees/second
*/
void setSpeed(arduinoSerial& Serial, int speed = SPEED_START, bool noset = false){
    speed = speed >= 120 ? 120 : speed;
    speed = speed <= 10 ? 10 : speed;
    if(!noset){
        curr_speed = speed;
    }
    Serial.print("SPEED " + std::to_string(speed) + "\n");
    std::cout << "setSpeed(): Set speed to " << speed << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_TIMEOUT));
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
    std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_TIMEOUT));
    char response[RESPONSE_MAX_SIZE] = {0};
    Serial.readBytesUntil('\n', response, RESPONSE_MAX_SIZE);
    std::cout << "homeArm(): Read response: " << response << std::endl;
}

void wave(arduinoSerial& Serial){
    std::vector<std::vector<double>> positions = {
        {157.5, 157/2, 200, 180, 167.5, 90},
        {157.5, 157/2, 100, 130, 167.5, 90},
        {157.5, 157/2, 200, 180, 167.5, 90},
        {157.5, 157/2, 100, 130, 167.5, 90},
        {157.5, 157/2, 200, 180, 167.5, 90},
        {157.5, 157/2, 100, 130, 167.5, 90},
        {157.5, 157/2, 200, 180, 167.5, 90},
        {157.5, 157/2, 100, 130, 167.5, 90},
        {157.5, 157.5, 187.5, 57.5, 157.5, 90}
    };
    for(auto p : positions){
        setJointPositions(Serial, p);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }
}

void pickUp(arduinoSerial& Serial){
    std::vector<std::vector<double>> positions = {
        {157.5, 157.5, 187.5, 57.5, 157.5, 90},
        {157.5, 100, 200, 100, 100, 90},
        {157.5, 100, 200, 100, 150, 90},
        {157.5, 100, 200, 100, 150, 120},
        {157.5, 100, 200, 100, 100, 120},
        {157.5, 157.5, 187.5, 57.5, 157.5, 90}
    };
    for(auto p : positions){
        setJointPositions(Serial, p);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }
}

void worm(arduinoSerial& Serial){
    std::vector<std::vector<double>> positions = {
        {157.5, 157.5, 187.5, 57.5, 157.5, 90},
        {157.5, 157.5, 200, 30, 167.5, 90},
        {157.5, 157.5, 100, 80, 167.5, 90},
        {157.5, 157.5, 200, 30, 167.5, 90},
        {157.5, 157.5, 100, 80, 167.5, 90},
        {157.5, 157.5, 200, 30, 167.5, 90},
        {157.5, 157.5, 100, 80, 167.5, 90},
        {157.5, 157.5, 200, 30, 167.5, 90},
        {157.5, 157.5, 100, 80, 167.5, 90},
        {157.5, 157.5, 187.5, 57.5, 157.5, 90}
    };
    for(auto p : positions){
        setJointPositions(Serial, p);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }
}
void chaos(arduinoSerial& Serial, bool death = false){
    setSpeed(Serial, death ? 120 : curr_speed, death);
    std::cout << (death ? "CHAOS IS COME, ": "") + std::string("FEAR THE ARM \n");
    std::vector<double> p;
    p.resize(6);
    for(int i=0; i<20; i++){
        for(int i=0; i<6; i++){
            p[i] = (double)(rand() % 150 + 50);
        }
        setJointPositions(Serial, p);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
    }
    setSpeed(Serial, curr_speed);
}
int main(){
    // Initialise serial communication
    arduinoSerial Serial; // Provides very arduino-like functions for interacting with a serial device
    Serial.openPort("/dev/ttyACM0"); // Default file for an arduino uno
    Serial.begin(BAUD_RATE);
    srand(time(NULL));
    bool limp = false;
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

    setSpeed(Serial);
    double pos_adjustment = POS_ADJUSTMENT_START;
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
            reset pos and speed
        */
        if (glfwGetKey(window, GLFW_KEY_APOSTROPHE) == GLFW_PRESS){
            std::cout << "Reset speed and adjustment\n";
            setSpeed(Serial);
            pos_adjustment = POS_ADJUSTMENT_START;
        }

        /*
            Per-joint control, a bit difficult to use but easy to implement
        */
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS){
            adjustJointPos(Serial, 1, pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS){
            adjustJointPos(Serial, 1, -pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
            adjustJointPos(Serial, 2, pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
            adjustJointPos(Serial, 2, -pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            adjustJointPos(Serial, 3, pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            adjustJointPos(Serial, 3, -pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            adjustJointPos(Serial, 4, pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            adjustJointPos(Serial, 4, -pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            adjustJointPos(Serial, 5, pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            adjustJointPos(Serial, 5, -pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
            adjustJointPos(Serial, 6, pos_adjustment);
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
            adjustJointPos(Serial, 6, -pos_adjustment);
        }

        /*
            Perform a wave :D
        */
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            std::cout << "Waving!\n";
            wave(Serial);
        }

        /*
            Pretend to pick something up
        */
        if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS){
            std::cout << "Pretending I am picking something up!\n";
            pickUp(Serial);
        }

        /*
            worm
        */
        if(glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS){
            std::cout << "Worm Time!\n";
            worm(Serial);
        }

        /*
            chaos
        */
        if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
            chaos(Serial, glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        }

        /*
            Adjust deg/sec speed
        */
        if(glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS){
            std::cout << "Speed down\n";
            setSpeed(Serial, curr_speed - SPEED_ADJUSTMENT);
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS){
            std::cout << "Speed up!\n";
            setSpeed(Serial, curr_speed + SPEED_ADJUSTMENT);
        }

        /*
            Adjust degree adjustment
        */
        if(glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS){
            std::cout << "Degree adjustment down, new adjustment: " << pos_adjustment - POS_ADJUSTMENT_ADJUSTMENT << "\n";
            pos_adjustment -= POS_ADJUSTMENT_ADJUSTMENT;
            pos_adjustment = std::max(pos_adjustment, 0.0);
            pos_adjustment = std::min(pos_adjustment, 45.0);
        }
        if(glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS){
            std::cout << "Degree adjustment up, new adjustment: " << pos_adjustment + POS_ADJUSTMENT_ADJUSTMENT << "\n";
            pos_adjustment += POS_ADJUSTMENT_ADJUSTMENT;
            pos_adjustment = std::max(pos_adjustment, 0.0);
            pos_adjustment = std::min(pos_adjustment, 45.0);
        }
        if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
            Serial.print(std::string("LIMP ") + (limp ? "0": "1") + "\n");
            limp = !limp;
            std::cout << "Toggled limp";
        }


        glfwSwapBuffers(window); // Render the current frame
    }

    // Cleanup
    glfwTerminate();
    return 0;
}
