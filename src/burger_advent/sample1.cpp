/**
 * @file sample1.cpp
 * @brief GLFW3サンプル.
 * @author tori31001 at gmail.com
 * Licensed  under Public Domain.
 *
 */
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <GL/glfw3.h>

static void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	std::cout << "key " << key << " action " << action << std::endl;
}

static void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	std::cout << "mouse_button" << " button " << button << " action " << action << std::endl;
}

static void cursor_pos_callback(GLFWwindow * window, double x, double y)
{
	std::cout << "cursor_pos" << " x " << x << " y " << y << std::endl;
}

static void window_size_callback(GLFWwindow * window, int width, int height)
{
	std::cout << "window_size" << " w " << width << " h " << height << std::endl;
}

static void window_close_callback(GLFWwindow * window)
{
	std::cout << "window_close" << std::endl;
}

int main(int argc, char** argv)
{
	// glfw初期化.
	if (!glfwInit()) {
		exit( EXIT_FAILURE );
	}
	// ウインドウの作成.
	GLFWwindow* window = glfwCreateWindow(800, 600, "burger_advent", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	// ウインドウをカレントコンテキストに設定.
	// この設定以降、OpenGL命令が使えます.
	glfwMakeContextCurrent(window);
	
	// glfwのコールバックの設定.
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetWindowSizeCallback(window, window_size_callback );
	glfwSetWindowCloseCallback(window, window_close_callback );
	glfwSwapInterval(1);

	// 背景色.
	glClearColor(0.21f, 0.21f, 0.21f, 1.0f);

	// メインループ.
	for (;;) 
	{
		// 描画処理.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glfwSwapBuffers(window);
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;
		if (glfwWindowShouldClose(window))
			break;
	}

	// 終了処理.
	window_close_callback(window);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit( EXIT_SUCCESS );
	return 0;
}

