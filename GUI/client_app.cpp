#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#define PORT 28009
#define BUFFER_SIZE 10

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    if (!glfwInit())
    {
        return -1;
    }
    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow *window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Init imgui
    //  Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    /* Loop until the user closes the window */
    ImGui::GetIO().FontGlobalScale = 2;

    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport();


        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            static int addressValue = 0;
            static int dataValue = 0;

            ImGui::Begin("Sender", NULL); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Address");
            ImGui::SameLine();
            ImGui::InputInt("##Address", &addressValue, 0);
            ImGui::Text("Data   ");
            ImGui::SameLine();
            ImGui::InputInt("##Data", &dataValue, 0);
            if (ImGui::Button("Send command"))
            {
                // Tao socket
                if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                {
                    std::cerr << "Socket creation error\n";
                    return -1;
                }

                // config
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(PORT);
                if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
                {
                    std::cerr << "Invalid address/ Address not supported\n";
                    return -1;
                }
                unsigned int unsignedAddressValue = (unsigned int)addressValue;
                unsigned int unsignedDataValue = (unsigned int)dataValue;
                // Ket noi den server
                if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                {
                    std::cerr << "Connection failed\n";
                }
                else
                {
                    snprintf(buffer, sizeof(buffer), "%u%u", unsignedAddressValue, unsignedDataValue); // Convert integer to string
                    send(sock, buffer, strlen(buffer), 0);
                    std::cout << "Sent: " << buffer << "\n";
                }
            }


            
            
            ImGui::End();

            ImGui::Begin("Reciever", NULL);
            ImGui::Text("Demo nhan du lieu tu DALI driver");
            ImGui::End();
        }

    

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Close the socket
    close(sock);
    glfwTerminate();
    return 0;
}
