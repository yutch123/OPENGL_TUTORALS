#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "Shader.h"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height); // объявление функции
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

bool dragging = false;

glm::vec3 lastPos;
glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

// единый источник правды для зума
float fov = 45.0f; // поле зрения
const GLfloat minFov = 15.0f;
const GLfloat maxFov = 90.0f;

const GLfloat zoomSpeed = 40.0f; // градусов в секунду

// Arcball-rotation
glm::vec3 screenToArcball(float x, float y) // преобразуем 2D-позицию курсора мыши (в пикселях) 
// → в 3D-вектор на поверхности виртуальной сферы
{
	// переводим координаты курсора из пикселей в нормализованное простарнство [-1;1]
	float nx = (2.0f * x - SCR_WIDTH) / SCR_WIDTH;
	float ny = (SCR_HEIGHT - 2.0f * y) / SCR_HEIGHT;

	float length2 = nx * nx + ny * ny; // вычисление квадрата длины вектора

	if (length2 <= 1.0f) 
		return glm::vec3(nx, ny, sqrt(1.0f - length2)); // на сфере
	else 
		return glm::normalize(glm::vec3(nx, ny, 0.0f)); // вне сферы
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
// 1 аргумент - указатель на окно GLFW
// 2 аргумент - какая кнопка мыши нажата 
// 3 аргумент - сотбытие: GLFW_PRESS (нажатие) или GLFW_RELEASE (отпускание)
// 4 аргумент - модификаторы клавиатуры (тут не используются)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) // обработка ЛКМ
	{
		if (action == GLFW_PRESS) // ЛКМ нажата
		{
			dragging = true; // флаг, что сейчас идет процесс перетаскивания
			double x, y; 
			glfwGetCursorPos(window, &x, &y); // получаем текущие координаты курсора мыши
			lastPos = screenToArcball((float)x, float(y)); // преобразуем 2D-координаты мыши в 3D-точку на единичной сфере (Arcball)
			// которая будет использоваться для вычисления вращения.
		}
		else if(action == GLFW_RELEASE) // Когда пользователь отпускает левую кнопку мыши, мы останавливаем процесс вращения.
		{
			dragging = false;
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!dragging) // если пользователь не держит ЛКМ, то вращение не происходит
		return;

	glm::vec3 currPos = screenToArcball((float)xpos, (float)ypos); // преобразование курсора в точку на сфере

	float dot = glm::clamp(glm::dot(lastPos, currPos), -1.0f, 1.0f);
	// lastpos - точка на сфере на последнем кадре (или при нажатии кнопки мыши)
	// dot - скалярное произведение двух векторов, показывает косинус угла между ними
	// glm::clamp - защита от погрешности вычислений
	float angle = acos(dot);  // угол между векторами на сфере

	glm::vec3 axis = glm::cross(lastPos, currPos); // вычисление оси вращение, axis - вектор, перпендикулярный 
	// lastPos и currPos.

	// применение вращения через кватернион
	if (glm::length(axis) > 0.0001f) // проверка, что ось не слишком мала
	{
		axis = glm::normalize(axis); // нормализация оси
		glm::quat delta = glm::angleAxis(angle * 2.0f, axis); // создаем кватернион для вращения на вычисленный угол вокруг оси
		rotation = delta * rotation; // обновляем общую ориентацию объекта
	}

	lastPos = currPos; // обновление последеней позиции
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
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// получаем и сохраняем адреса функций OpenGL в памяти.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Не удалось инициализировать GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader ourShader("3.3.shader.vs", "3.3.shader.fs");

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



	unsigned int VBO; // объявляем VBO
	unsigned int VAO; // объявляем VAO

	glGenBuffers(1, &VBO); // создаем буферный объект с идентификатором 1
	glGenVertexArrays(1, &VAO); // создаем массив вершин

	glBindVertexArray(VAO); // привязываем VAO

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // делаем VBO текущим буфером для цели GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // функция предназначенная для копирования пользовательских данных в связанный в данный момент буфер
	/* 1 аргумент функции - это тип буфера, в который мы хотим скопировать данные
	*  2 аргумент указывает размер данных в байтах
	*  3 аргумент фактические данные, которые мы хотим отправить
	*  4 аргумент способ обработки заданных данных */

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0); // хранение данных в VBO
	// 1 параметр - какой вершинный атрибут мы хотим настроить
	// 2 параметр - размер вершинного атрибута
	// 3 параметр - тип данных
	// 4 параметр - нормализация данных,  Если мы вводим целочисленные типы данных (int, byte) и устанавливаем для этого параметра значение GL_TRUE, целочисленные данные нормализуются до 0 (или -1 для знаковых данных)
	// и 1 при преобразовании в число с плавающей запятой. Для нас это неактуально, поэтому оставим значение GL_FALSE.
	// 5 параметр - шаг, сообщает нам расстояние между последовательными атрибутами вершин.
	// 6 параметр - смещение, где в буфере начинаются данные о местоположении.
	glEnableVertexAttribArray(0); // активируем атрибут позиции (location = 0)

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

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
	unsigned char* data = stbi_load("Texture.png", &width, &height, &nrChannels, 0); // Сначала функция принимает в качестве входных данных
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

	data = stbi_load("AUDUMBLA.png", &width, &height, &nrChannels, 0);
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

	while (!glfwWindowShouldClose(window)) // проверяем, нужно ли окну закрыться
	{
		processInput(window); // вызываем процесс ввода на каждой итерации цикла рендеринга

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // очищаем экран выбрав нужный цвет
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // очищаем только цветной буфер

		// Включаем каркасный режим
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		// создание матрицы трансформации с Arcball вращением
		
		// MODEL - только arcball + масштаб
		glm::mat4 model = glm::mat4(1.0f);
		model *= glm::toMat4(rotation);
		model = glm::scale(model, glm::vec3(0.5f));

		// VIEW - камера (статичная)
		glm::mat4 view = glm::lookAt(
			glm::vec3(0.0f, 0.0f, 3.0f), // позиция камеры
			glm::vec3(0.0f, 0.0f, 0.0f), // направление камера
			glm::vec3(0.0f, 1.0f, 0.0f) // вверх камеры
		);

		// PROJECTION - перспектива
		glm::mat4 projection = glm::perspective(
			glm::radians(fov), (GLfloat)SCR_WIDTH / (GLfloat)SCR_HEIGHT,
			0.1f,
			100.0f
		);

		// передача в шейдер
		ourShader.use();
		ourShader.setMat4("model", model);
		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		glBindVertexArray(VAO); // как только мы захотим нарисовать объект, мы просто привязываем VAO  к предпочтительным настройкам перед рисованием объекта
		glDrawArrays(GL_TRIANGLES, 0, 36); // рисуем 2 треугольника
		// 1 аргумент - тип примитива
		// 2 аргумент - кол-во элементов
		// 3 аргумент - тип индексов, который имеет вид GL_UNSIGNED_INT

		glfwSwapBuffers(window); // меняет местами цветовой буфер
		glfwPollEvents(); // проверка события, например ввод с клавиатуры
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

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

void framebuffer_size_callback(GLFWwindow* window, int SCR_WIDTH, int SCR_HEIGHT) // функция для изменения размера окна экрана 
{
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT); // задаем область просмотра 0,0 - начальная точка, width, height - конечная
}
