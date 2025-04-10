#!/bin/bash

g++ -O1 client_app.cpp imgui_demo.cpp imgui_draw.cpp imgui_impl_glfw.cpp imgui_impl_opengl2.cpp imgui_tables.cpp imgui_widgets.cpp imgui.cpp -lGL -lglfw