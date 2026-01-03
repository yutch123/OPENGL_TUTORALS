#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtx/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "imgui.h"                   // основной интерфейс ImGui
#include "imgui_impl_glfw.h"         // backend для GLFW
#include "imgui_impl_opengl3.h"     // backend для OpenGL3
#include "imconfig.h"

#include "Shader.h"
#include "stb_image.h"
#include "Arcball.h"
#include <EditorUI.h>

#include "Mesh.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height); // объявление функции
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int gWidth = SCR_WIDTH;
int gHeight = SCR_HEIGHT;

// единый источник правды для зума
float fov = 45.0f; // поле зрения
const GLfloat minFov = 15.0f;
const GLfloat maxFov = 90.0f;
const GLfloat zoomSpeed = 10.0f; // градусов в секунду

Arcball arcball(SCR_WIDTH, SCR_HEIGHT);

// состояние выбранного примитива
PrimitiveType currentPrimitive = PrimitiveType::None;

void renderPrimitive(Shader& shader, unsigned int VAO, Arcball& arcball, unsigned int vertexCount, unsigned int texture1, unsigned int texture2)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2);

	// создание матрицы трансформации с Arcball вращением

	// MODEL - только arcball + масштаб
	glm::mat4 model = glm::mat4(1.0f);
	model *= arcball.getRotationMatrix();
	model = glm::scale(model, glm::vec3(0.5f));

	// VIEW - камера (статичная)
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 3.0f), // позиция камеры
		glm::vec3(0.0f), // направление камера
		glm::vec3(0.0f, 1.0f, 0.0f) // вверх камеры
	);

	// вычисляем aspect локально, чтобы защитить от 0 высоты
	float aspect = (gHeight == 0) ? 1.0f : (float)gWidth / (float)gHeight;

	// PROJECTION - перспектива
	glm::mat4 projection = glm::perspective(
		glm::radians(fov),
		aspect,
		0.1f,
		100.0f
	);

	// передача в шейдер
	shader.use();
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	glBindVertexArray(VAO); // как только мы захотим нарисовать объект, мы просто привязываем VAO  к предпочтительным настройкам перед рисованием объекта
	glDrawArrays(GL_TRIANGLES, 0, vertexCount); // рисуем 2 треугольника
	// 1 аргумент - тип примитива
	// 2 аргумент - кол-во элементов
	// 3 аргумент - тип индексов, который имеет вид GL_UNSIGNED_INT
	glBindVertexArray(0); // отвязываем после рисвоания
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// yoffset > 0 означает прокрутку вверх (zoom in), yoffset < 0 — вниз (zoom out)
	fov -= (float)yoffset * zoomSpeed * 0.1f; // 0.1f — чувствительность прокрутки
	fov = glm::clamp(fov, minFov, maxFov);
}

int main()
{
	glfwInit(); // инициализируем GLFW
	// настриваем GLFW с помощью glfwInit
	// первый аргумент glfwWindowHint сообщает нам, какую опцию мы хотим настроить
	// второй аргумент это целое число, которое задает значение нашей опции
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

	// создаем объект окна
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AUDUMBLA", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Не удалось создать окно GLFW" << std::endl;
		glfwTerminate(); // завершаем библеотеку
		return -1;
	}

	glfwMakeContextCurrent(window); // все следующие команды, которые будут вызваны относятся к этому окну.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// получаем и сохраняем адреса функций OpenGL в памяти.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Не удалось инициализировать GLAD" << std::endl;
		return -1;
	}

	// Инициализация окна
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	(void)io;
	ImGui::StyleColorsClassic();

	// Инициализация бэкендов
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// UI слой

	EditorUI editorUI;

	glEnable(GL_DEPTH_TEST);

	Shader ourShader("shaders/3.3.shader.vs", "shaders/3.3.shader.fs");

	GLfloat vertices[] = {
		// первый треугольник //текстуры
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	// Пирамида

	GLfloat pyramidVertices[] = {
		// Пирамида с квадратным основанием, вершина в (0,1,0)
		// Вершина
		 0.0f,  1.0f,  0.0f,  0.5f, 1.0f, // верх
		-0.5f,  0.0f,  0.5f,  0.0f, 0.0f, // передний левый
		 0.5f,  0.0f,  0.5f,  1.0f, 0.0f, // передний правый

		 0.0f,  1.0f,  0.0f,  0.5f, 1.0f, // верх
		 0.5f,  0.0f,  0.5f,  1.0f, 0.0f, // передний правый
		 0.5f,  0.0f, -0.5f,  1.0f, 1.0f, // задний правый

		 0.0f,  1.0f,  0.0f,  0.5f, 1.0f,
		 0.5f,  0.0f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.0f, -0.5f,  0.0f, 1.0f,

		 0.0f,  1.0f,  0.0f,  0.5f, 1.0f,
		-0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.0f,  0.5f,  0.0f, 0.0f,

		// основание (квадрат)
		-0.5f,  0.0f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.0f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.0f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.0f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.0f, -0.5f,  0.0f, 1.0f,
		-0.5f,  0.0f,  0.5f,  0.0f, 0.0f
	};

	Mesh cube = createMesh(vertices, sizeof(vertices) / sizeof(float));
	Mesh pyramid = createMesh(pyramidVertices, sizeof(pyramidVertices) / sizeof(float));

	unsigned int texture1, texture2;
	glGenTextures(1, &texture1); // принимает на вход количество текстур, которые мы хотим сгенерировать и сохраняет их в массиве texture
	glBindTexture(GL_TEXTURE_2D, texture1); // привязываем текстуру
	// теперь мы можем начать создавать текстуру, используя ранее загруженные данные
	// glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	// 1 аргумент - цель текстуры. Если указать GL_TEXTURE_2D, то эта операция создаст текстуру для связанного в данный момент объекта текстуры с тем же целевым объектом
	// (то есть текстуры, связанные с целевыми объектами GL_TEXTURE_1D или GL_TEXTURE_3D , не будут затронуты).
	// 2 аргумент указывает на уровень MIP-карты, для которого мы хотим создать текстуру. Если вы хотите задать каждый уровень MIP-карты вручную, оставьте значение на базовом уровне 0.
	// 3 аргумент сообщает OpenGL, в каком формате мы хотим сохранить текстуру. В нашем изображении есть только RGB значения,
	// поэтому мы сохраним текстуру с RGB значениями.
	// 4 и 5 аргументы задают ширину и высоту результирующей текстуры.
	// 6 аргумент - всегда должен быть 0.
	// 7 и 8 аргументы задают формат и тип данных исходного изображения.
	// Последний аргумент - фактические данные изображения.

	// задаем параметры наложения/фильтрации текстуры (на привязанном в данный момент текстурном объекте).
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // поведение текстуры на оси X, если координата текстуры u 
	// выходит за диапозон [0, 1], то текстура повторяется.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // поведение текстуры на оси Y, если координата текстуры u 
	// выходит за диапозон [0, 1], то текстура повторяется.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // фильтрация при уменьшении текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // фильтрация при увелечении текстуры
	// загрузка и генерация текстуры
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("assets/texture/Texture.png", &width, &height, &nrChannels, 0); // Сначала функция принимает в качестве входных данных
	// расположение файла изображения. Затем она ожидает, что вы укажете три ints в качестве второго, третьего и четвёртого аргумента
	// которые stb_image.h заполнят шириной, высотой и количеством цветовых каналов полученного изображения.
	if (data) // проверка загрузки изображения
	{
		GLenum format;
		if (nrChannels == 1) format = GL_RED;
		else if (nrChannels == 3) format = GL_RGB;
		else if (nrChannels == 4) format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		// Создает или перезаписывает изображение текстуры и копирует данные из CPU-памяти в видеопамять (GPU) для текущей привязанной текстуры.
		glGenerateMipmap(GL_TEXTURE_2D); // генерируем все уровни mipmap
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	// texture2
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("assets/texture/AUDUMBLA.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format;
		if (nrChannels == 1) format = GL_RED;
		else if (nrChannels == 3) format = GL_RGB;
		else if (nrChannels == 4) format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	ourShader.use();
	glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
	ourShader.setInt("texture2", 1);

	// основной цикл рендеринга

	while (!glfwWindowShouldClose(window)) // проверяем, нужно ли окну закрыться
	{
		processInput(window); // вызываем процесс ввода на каждой итерации цикла рендеринга

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // очищаем экран выбрав нужный цвет
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очищаем только цветной буфер

		// Включаем каркасный режим
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Вызываем отрисовку куба
		// renderPrimitive(ourShader, VAO, arcball, texture1, texture2);

		// Рендер ImGui
		editorUI.beginFrame();
		editorUI.render();

		PrimitiveType requestedPrimitive = editorUI.consumePrimitiveRequest();
		if (requestedPrimitive != PrimitiveType::None)
		{
			currentPrimitive = requestedPrimitive; // обновляем текущее состояние
		}

		switch (currentPrimitive)
		{
		case PrimitiveType::Cube:
			renderPrimitive(ourShader, cube.VAO, arcball, cube.vertexCount, texture1, texture2);
			break;
		case PrimitiveType::Pyramid:
			renderPrimitive(ourShader, pyramid.VAO, arcball, pyramid.vertexCount, texture1, texture2);
			break;
		default:
			break;
		}

		glfwSwapBuffers(window); // меняет местами цветовой буфер
		glfwPollEvents(); // проверка события, например ввод с клавиатуры
	}

	glfwTerminate(); // удаляем все выделенные ресурсы
	return 0;
}

void processInput(GLFWwindow *window)
{
	static GLfloat lastTime = glfwGetTime();
	GLfloat currentTime = glfwGetTime();
	GLfloat deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// бооквая кнопка "назад" → zoom in
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_4) == GLFW_PRESS)
		fov -= zoomSpeed * deltaTime;

	// боковая кнопка "вперед" → zoom out
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_5) == GLFW_PRESS)
		fov += zoomSpeed * deltaTime;

	// Ctrl + / Ctrl -
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
			fov -= zoomSpeed * deltaTime;

		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
			fov += zoomSpeed * deltaTime;
	}

	fov = glm::clamp(fov, minFov, maxFov);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) // функция для изменения размера окна экрана 
{
	gWidth = width;
	gHeight = (height == 0) ? 1 : height;
	glViewport(0, 0, width, height); // задаем область просмотра 0,0 - начальная точка, width, height - конечная
	arcball.onResize(gWidth, gHeight);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	arcball.onMouseButton(button, action, x, y);
}

void cursor_position_callback(GLFWwindow*, double x, double y)
{
	arcball.onCursorMove(x, y);
}


