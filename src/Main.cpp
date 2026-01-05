// Включает экспериментальные расширения GLM 
// (например, дополнительные утилиты для кватернионов)
#define GLM_ENABLE_EXPERIMENTAL

// =======================
// GLM — математика
// =======================

// Базовые типы GLM: vec*, mat*, операции
#include <glm/glm.hpp>
// Утилиты для матриц преобразований (translate, rotate, scale, perspective и т.д.)
#include <glm/gtc/matrix_transform.hpp>
// Доступ к данным GLM-типов в виде указателей (нужно для glUniform*)
#include <glm/gtc/type_ptr.hpp>
// Базовая поддержка кватернионов
#include <glm/gtc/quaternion.hpp>

// Расширенные операции с кватернионами (преобразования, интерполяции)
#include <glm/gtx/quaternion.hpp>

// =======================
// OpenGL / Windowing
// =======================

// Загрузчик функций OpenGL (обязателен для core-профиля)
#include <glad/glad.h>
// GLFW — создание окна, контекст OpenGL, ввод (клавиатура, мышь)
#include <GLFW/glfw3.h>

// =======================
// Стандартная библиотека
// =======================

// Потоковый ввод/вывод (логирование, отладка)
#include <iostream>

// =======================
// ImGui — пользовательский интерфейс
// =======================

// Основной заголовок Dear ImGui
#include "imgui.h" 
// Backend ImGui для GLFW (ввод, окно)
#include "imgui_impl_glfw.h"
// Backend ImGui для OpenGL (рендеринг UI)
#include "imgui_impl_opengl3.h"
// Конфигурация ImGui (опциональные настройки, макросы)
#include "imconfig.h"

// =======================
// Внутренние модули проекта
// =======================

// Обертка над OpenGL shader program (компиляция, линковка, uniform'ы)
#include "Shader.h"
// stb_image — загрузка изображений (текстуры)
#include "stb_image.h"
// Arcball — логика вращения камеры/объекта с помощью мыши
#include "Arcball.h"
// EditorUI — слой пользовательского интерфейса (ImGui)
// Управляет параметрами сцены, но не рендерингом
#include <EditorUI.h>

// =======================
// Геометрия / примитивы
// =======================

// Mesh — базовый класс для работы с VAO/VBO/EBO и отрисовки геометрии
#include "Mesh.h"
// Sphere — примитив сферы (генерация вершин и индексов)
// Обычно наследуется от Mesh или использует его
#include "Sphere.h"

// Assimp

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Windows API

#include <Windows.h>

void assimpTest()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		"assets/models/test.obj",
		aiProcess_Triangulate |
		aiProcess_FlipUVs
	);

	if (!scene)
	{
		OutputDebugStringA(importer.GetErrorString());
	}
}

// =======================
// GLFW callbacks — объявления
// =======================

// Callback, вызываемый GLFW при изменении размеров окна.
// Используется для обновления viewport'а OpenGL и хранения актуальных размеров экрана.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// Callback обработки нажатий кнопок мыши.
// Как правило, используется для начала/окончания взаимодействий (например, Arcball).
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
// Callback движения курсора мыши.
// Применяется для интерактивного управления камерой или объектом (вращение, drag).
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
// Функция обработки ввода с клавиатуры.
// Обычно вызывается каждый кадр в render loop.
void processInput(GLFWwindow *window);

// =======================
// Размеры окна (константы)
// =======================


// Начальная ширина окна приложения
const unsigned int SCR_WIDTH = 800;
// Начальная высота окна приложения
const unsigned int SCR_HEIGHT = 600;

// =======================
// Текущее состояние окна
// =======================

// Глобальные переменные, хранящие актуальные размеры framebuffer'а.
// Обновляются в framebuffer_size_callback.
// Используются для корректных вычислений aspect ratio и матриц проекции.
int gWidth = SCR_WIDTH;
int gHeight = SCR_HEIGHT;

// =======================
// Параметры камеры / зума
// =======================

// Единый источник правды для поля зрения камеры (Field of View).
// Используется при построении матрицы проекции.
float fov = 45.0f;

// Минимально допустимое поле зрения (максимальный зум)
const GLfloat minFov = 15.0f;

// Максимально допустимое поле зрения (минимальный зум)
const GLfloat maxFov = 90.0f;

// Скорость изменения FOV (градусов в секунду).
// Удобно использовать при плавном зуме.
const GLfloat zoomSpeed = 10.0f;

// =======================
// Arcball — управление вращением
// =======================

// Экземпляр Arcball для вращения сцены или объекта мышью.
// Инициализируется начальными размерами экрана,
// что необходимо для корректного проецирования координат курсора.
Arcball arcball(SCR_WIDTH, SCR_HEIGHT);

// =======================
// Состояние сцены
// =======================

// Текущий выбранный примитив сцены.
// Используется для определения того, какой объект нужно создавать/рендерить.
// PrimitiveType::None означает, что примитив не выбран.
PrimitiveType currentPrimitive = PrimitiveType::None;

void renderPrimitive(Shader& shader, unsigned int VAO, Arcball& arcball, bool useIndices, unsigned int count, unsigned int texture1)
{
	// =======================
	// Текстура
	// =======================

	// Активируем текстурный юнит 0
	glActiveTexture(GL_TEXTURE0); 
	// Привязываем 2D-текстуру к текущему юниту
	glBindTexture(GL_TEXTURE_2D, texture1);

	// =======================
	// Model matrix (объект)
	// =======================

	// Единичная матрица — базовое состояние без трансформаций
	glm::mat4 model = glm::mat4(1.0f);
	// Применяем вращение, полученное от Arcball (интерактивное вращение мышью)
	model *= arcball.getRotationMatrix();
	// Масштабирование объекта (сейчас 1.0 — без изменения размера)
	model = glm::scale(model, glm::vec3(1.0f));

	// =======================
	// View matrix (камера)
	// =======================

	// Простая камера, смотрящая из точки (0, 0, 3) в начало координат
	glm::mat4 view = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 3.0f), // позиция камеры
		glm::vec3(0.0f), // точка, куда смотрим
		glm::vec3(0.0f, 1.0f, 0.0f) // вектор "вверх"
	);

	// =======================
	// Projection matrix
	// =======================

	 // Защита от деления на ноль при сворачивании окна
	float aspect = (gHeight == 0) ? 1.0f : (float)gWidth / (float)gHeight;

	// Перспективная проекция
	glm::mat4 projection = glm::perspective(
		glm::radians(fov), // поле зрения (зум)
		aspect, // соотношение сторон окна
		0.1f, // ближняя плоскость отсечения
		100.0f // дальняя плоскость отсечения
	);

	// =======================
	// Передача данных в шейдер
	// =======================

	// Активация shader program
	shader.use();
	// Позиция источника света в мировом пространстве
	shader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));

	// Позиция камеры (используется для расчёта освещения)
	shader.setVec3("viewPos", glm::vec3(0.0f, 0.0f, 3.0f));

	// Матрицы преобразований
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	// =======================
	// Отрисовка геометрии
	// =======================

	// Привязываем VAO примитива
	glBindVertexArray(VAO); 

	// Выбор способа отрисовки:
	// - glDrawElements → индексированная геометрия (EBO)
	// - glDrawArrays   → неиндексированная геометрия

	if (useIndices)
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(GL_TRIANGLES, 0, count);

	// =======================
	// Очистка состояния
	// =======================

	// Отвязываем VAO
	glBindVertexArray(0);

	// Отвязываем текстуру
	glBindTexture(GL_TEXTURE_2D, 0);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// GLFW передаёт смещение колеса мыши:
	// yoffset > 0  → прокрутка вверх
	// yoffset < 0  → прокрутка вниз
	//
	// Изменяем поле зрения (FOV), реализуя эффект зума камеры.
	// Чем меньше FOV — тем сильнее приближение.
	fov -= (float)yoffset * zoomSpeed * 0.1f;

	// Ограничиваем FOV допустимыми значениями,
	// чтобы избежать искажений перспективы и артефактов.
	fov = glm::clamp(fov, minFov, maxFov);
}

int main()
{
	// Инициализация GLFW — библиотеки для создания окна и работы с контекстом OpenGL
	glfwInit();
	// Устанавливаем требуемую версию OpenGL (3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	// Указываем использование Core Profile (без устаревших функций)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

	// Создаем окно с указанными размерами и заголовком
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DEMO", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Не удалось создать окно GLFW" << std::endl;
		glfwTerminate(); // // Завершаем работу с GLFW при ошибке
		return -1;
	}

	glfwMakeContextCurrent(window); // Делаем созданное окно текущим для OpenGL
	// Устанавливаем колбэки для обработки изменения размера окна, мыши и скролла
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Инициализация GLAD для получения указателей на функции OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Не удалось инициализировать GLAD" << std::endl;
		return -1;
	}

	// Инициализация ImGui для графического интерфейса
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	(void)io;
	ImGui::StyleColorsClassic();

	// Инициализация бэкендов ImGui для GLFW и OpenGL
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Создаем UI слой для редактора
	EditorUI editorUI;

	// Включаем тест глубины, чтобы корректно отображались пересекающиеся объекты
	glEnable(GL_DEPTH_TEST);

	// Создаем шейдер и устанавливаем текстурный слот
	Shader ourShader("shaders/3.3.shader.vs", "shaders/3.3.shader.fs");
	ourShader.use();
	ourShader.setInt("texture1", 0);

	// Создаем вершины для куба (позиция, нормаль, текстурные координаты)
	std::vector<Vertex> cubeVertices = {

	// Передняя грань (z = -0.5)
	Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}),
	Vertex({ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}),
	Vertex({ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}),
	Vertex({ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}),
	Vertex({-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}),
	Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}),

	// Задняя грань (z = 0.5)
	Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}),
	Vertex({ 0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}),
	Vertex({ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}),
	Vertex({ 0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}),
	Vertex({-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}),
	Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}),

	// Левая грань (x = -0.5)
	Vertex({-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
	Vertex({-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),
	Vertex({-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),
	Vertex({-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),
	Vertex({-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
	Vertex({-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),

	// Правая грань (x = 0.5)
	Vertex({ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
	Vertex({ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),
	Vertex({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),
	Vertex({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),
	Vertex({ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),
	Vertex({ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),

	// Нижняя грань (y = -0.5)
	Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}),
	Vertex({ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}),
	Vertex({ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}),
	Vertex({ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}),
	Vertex({-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
	Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}),

	// Верхняя грань (y = 0.5)
	Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}),
	Vertex({ 0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}),
	Vertex({ 0.5f, 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}),
	Vertex({ 0.5f, 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}),
	Vertex({-0.5f, 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}),
	Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}),
	};

	// Пирамида

	std::vector<Vertex> pyramidVertices = {
		// Вершина
		Vertex({ 0.0f,  0.5f,  0.0f}, {0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}),
		// Передний левый
		Vertex({-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}),
		// Передний правый
		Vertex({ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}),
		// Правая грань
		Vertex({ 0.0f,  0.5f,  0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 1.0f}),
		Vertex({ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}),
		Vertex({ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}),

		// Задняя грань
		Vertex({ 0.0f,  0.5f,  0.0f}, {0.0f, 0.0f, -1.0f}, {0.5f, 1.0f}),
		Vertex({ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}),
		Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}),

		// Левая грань
		Vertex({ 0.0f,  0.5f,  0.0f}, {-1.0f, 0.0f, 0.0f}, {0.5f, 1.0f}),
		Vertex({-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}),
		Vertex({-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}),

		// Основание (два треугольника)
		Vertex({-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
		Vertex({ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}),
		Vertex({ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}),

		Vertex({ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}),
		Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}),
		Vertex({-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}),
	};

	// Создаем меши для рендеринга примитивов
	Mesh cube = createMesh(cubeVertices);
	Mesh pyramid = createMesh(pyramidVertices);
	Sphere sphere(0.5f, 36, 18); // Создаем сферу (радиус, количество секторов, стэков)

	// Генерация и настройка текстуры
	unsigned int texture1;
	glGenTextures(1, &texture1); // Создаем объект текстуры
	glBindTexture(GL_TEXTURE_2D, texture1); // Привязываем текстуру

	// Настройка параметров оборачивания и фильтрации текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Загрузка изображения с диска с помощью stb_image
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("assets/texture/Texture.png", &width, &height, &nrChannels, 0);
	if (data) 
	{
		GLenum format;
		if (nrChannels == 1) format = GL_RED;
		else if (nrChannels == 3) format = GL_RGB;
		else if (nrChannels == 4) format = GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // Генерация MIP-карт
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data); // Освобождаем память изображения

	// Основной цикл рендеринга
	while (!glfwWindowShouldClose(window)) 
	{
		processInput(window); // Обработка пользовательского ввода

		// Очистка цветового и глубинного буферов
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		// Включаем каркасный режим
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// Вызываем отрисовку куба
		// renderPrimitive(ourShader, VAO, arcball, texture1, texture2);

		// Рендеринг ImGui
		editorUI.beginFrame();
		editorUI.render();

		// Получаем текущий запрос на примитив из UI
		PrimitiveType requestedPrimitive = editorUI.consumePrimitiveRequest();
		if (requestedPrimitive != PrimitiveType::None)
		{
			currentPrimitive = requestedPrimitive; 
		}

		// Рендерим выбранный примитив
		switch (currentPrimitive)
		{
		case PrimitiveType::Cube:
			renderPrimitive(ourShader, cube.VAO, arcball, false, cube.vertexCount, texture1);
			break;
		case PrimitiveType::Pyramid:
			renderPrimitive(ourShader, pyramid.VAO, arcball, false, pyramid.vertexCount, texture1);
			break;
		case PrimitiveType::Sphere:
			renderPrimitive(ourShader, sphere.VAO, arcball, true, sphere.indexCount, texture1);
			break;
		default:
			break;
		}

		glfwSwapBuffers(window); // Меняем цветовые буферы местами
		glfwPollEvents(); // Обрабатываем события ввода
	}

	// Завершаем работу GLFW и освобождаем ресурсы
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	// =======================
	// Расчёт deltaTime
	// =======================

	// Время предыдущего кадра хранится как static,
	// чтобы сохраняться между вызовами функции.
	static GLfloat lastTime = glfwGetTime();

	// Текущее время (в секундах с момента инициализации GLFW)
	GLfloat currentTime = glfwGetTime();

	// Разница между текущим и предыдущим кадром
	GLfloat deltaTime = currentTime - lastTime;

	// Обновляем время последнего кадра
	lastTime = currentTime;

	// =======================
	// Обработка клавиатуры
	// =======================

	 // Клавиша ESC — закрытие окна приложения
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// =======================
	// Управление зумом мышью
	// =======================

	// Боковая кнопка мыши "назад" → приближение (уменьшение FOV)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_4) == GLFW_PRESS)
		fov -= zoomSpeed * deltaTime;

	// Боковая кнопка мыши "вперёд" → отдаление (увеличение FOV)
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_5) == GLFW_PRESS)
		fov += zoomSpeed * deltaTime;

	// =======================
	// Управление зумом с клавиатуры
	// =======================

	// Модификатор Ctrl + клавиши увеличения/уменьшения
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
		glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
	{
		// Ctrl + '=' → zoom in
		if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
			fov -= zoomSpeed * deltaTime;

		// Ctrl + '-' → zoom out
		if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
			fov += zoomSpeed * deltaTime;
	}

	// =======================
	// Ограничение FOV
	// =======================

	// Ограничиваем поле зрения допустимым диапазоном,
	// чтобы избежать искажений перспективы и численных проблем.

	fov = glm::clamp(fov, minFov, maxFov);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) // функция для изменения размера окна экрана 
{
	// Сохраняем актуальную ширину окна.
	// Используется для расчёта aspect ratio и матрицы проекции.
	gWidth = width;

	// Защита от нулевой высоты (например, при сворачивании окна),
	// чтобы избежать деления на ноль при вычислении aspect ratio.
	gHeight = (height == 0) ? 1 : height;

	// Устанавливаем viewport OpenGL:
	// (0, 0)           — нижний левый угол окна
	// (width, height) — размеры области отрисовки
	//
	// Все последующие вызовы рендера будут происходить в этой области.
	glViewport(0, 0, width, height);

	// Сообщаем Arcball'у новые размеры экрана,
	// чтобы корректно преобразовывать координаты курсора
	// в нормализованное пространство вращения.
	arcball.onResize(gWidth, gHeight);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int)
{
	// Текущие координаты курсора в момент нажатия/отпускания кнопки мыши.
	// GLFW возвращает координаты в экранном пространстве окна.
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// Передаём событие в Arcball:
	//  - button  → какая кнопка мыши была нажата
	//  - action  → нажатие или отпускание (GLFW_PRESS / GLFW_RELEASE)
	//  - x, y    → позиция курсора в момент события
	//
	// Arcball самостоятельно:
	//  - определяет начало/конец вращения,
	//  - сохраняет стартовую позицию,
	//  - подготавливает данные для последующего onCursorMove().
	arcball.onMouseButton(button, action, x, y);
}

void cursor_position_callback(GLFWwindow*, double x, double y)
{
	// Передаём текущее положение курсора в Arcball.
	// Arcball самостоятельно:
	//  - отслеживает состояние drag'а,
	//  - преобразует координаты экрана в вектор на сфере,
	//  - вычисляет кватернион вращения.
	//
	// Callback не содержит логики вращения — он лишь транслирует событие.
	arcball.onCursorMove(x, y);
}


