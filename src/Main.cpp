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

// Assimp

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <Model.h>

unsigned int pickingFBO = 0;
unsigned int pickingTexture = 0;
unsigned int depthRenderBuffer = 0;
Shader* pickingShader = nullptr;

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

Model* loadedModel = nullptr; // указатель на модель

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

	ImFontGlyphRangesBuilder builder;
	builder.AddRanges(io.Fonts->GetGlyphRangesDefault());   // латиница и цифры
	builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());  // кириллица
	ImVector<ImWchar> ranges;
	builder.BuildRanges(&ranges);

	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 14.0f, nullptr, ranges.Data);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Создаем UI слой для редактора
	EditorUI editorUI;

	// Включаем тест глубины, чтобы корректно отображались пересекающиеся объекты
	glEnable(GL_DEPTH_TEST);

	// Создаем FBO для Color Picking
	glGenFramebuffers(1, &pickingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO); 

	// создаём текстуру для хранения цветов мешей
	glGenTextures(1, &pickingTexture);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

	// текстура глубины
	unsigned int depthRenderBuffer;
	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	// проверяем FBO
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Picking FBO is not complete!" << std::endl;

	// отключаем FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Создаем шейдер для Color Picking
	pickingShader = new Shader("shaders/picking.vs", "shaders/picking.fs");

	// Создаем шейдер и устанавливаем текстурный слот
	Shader ourShader("shaders/3.3.shader.vs", "shaders/3.3.shader.fs");
	ourShader.use();
	ourShader.setInt("texture1", 0);

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

		// 

		// Очистка цветового и глубинного буферов
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		// Используем шейдер
		ourShader.use();

		// Устанавливаем матрицы (view, projection, model)
		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)gWidth / gHeight, 0.1f, 100.0f);
		glm::mat4 model = arcball.getRotationMatrix();

		// применяем масштаб только если модель загружена
		if (loadedModel)
		{
			float s = loadedModel->getScale();
			if (s <= 0.0f) s = 1.0f; // защита от нуля
			model = glm::scale(model, glm::vec3(s));
		}

		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("model", model);

		// Привязываем текстуру перед отрисовкой
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);

		if (editorUI.loadModelRequested)
		{
			if (loadedModel) delete loadedModel;
			loadedModel = new Model("assets/models/Model3D.obj");

			glm::vec3 modelSize = loadedModel->getSize();
			float maxDimension = glm::max(glm::max(modelSize.x, modelSize.y), modelSize.z);
			if (maxDimension <= 0.0f) maxDimension = 1.0f;
			float scaleFactor = 1.0f / maxDimension;
			loadedModel->setScale(scaleFactor);

			editorUI.loadModelRequested = false;
		}

		if (loadedModel)
		{
			loadedModel->setRotationMatrix(arcball.getRotationMatrix());
			loadedModel->Draw(ourShader);
		}

		// Рендеринг ImGui
		editorUI.beginFrame();
		editorUI.render(loadedModel);

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

// Загрузка модели через Assimp


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
	glViewport(0, 0, gWidth, gHeight);

	// Сообщаем Arcball'у новые размеры экрана,
	// чтобы корректно преобразовывать координаты курсора
	// в нормализованное пространство вращения.
	arcball.onResize(gWidth, gHeight);

	// Пересоздаём Picking FBO

	glDeleteTextures(1, &pickingTexture);
	glDeleteRenderbuffers(1, &depthRenderBuffer);

	glGenTextures(1, &pickingTexture);
	glBindTexture(GL_TEXTURE_2D, pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, gWidth, gHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenRenderbuffers(1, &depthRenderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, gWidth, gHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Picking FBO is not complete after resize!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Вспомогательная функция для генерации уникального цвета по ID

glm::vec3 IDtoColor(unsigned int meshID)
{
	unsigned int r = (meshID + 1) & 0xFF;
	unsigned int g = ((meshID + 1) >> 8) & 0xFF;
	unsigned int b = ((meshID + 1) >> 16) & 0xFF;
	return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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

	// Обработка выбора меша при нажатии левой кнопки мыши
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		if (!loadedModel || !pickingShader) return;

		glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)gWidth / gHeight, 0.1f, 100.0f);
		glm::mat4 model = arcball.getRotationMatrix();
		if (loadedModel) model = glm::scale(model, glm::vec3(loadedModel->getScale()));

		// Привязываем FBO для Color Picking
		glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pickingShader->use();
		pickingShader->setMat4("view", view);
		pickingShader->setMat4("projection", projection);
		pickingShader->setMat4("model", model);

		loadedModel->drawForPicking(*pickingShader);

		// OpenGL считает координату Y от нижнего края, мышь — от верхнего
		int mouseX = static_cast<int>(x);
		int mouseY = gHeight - static_cast<int>(y);

		unsigned char data[3]; // RGB
		glReadPixels(mouseX, mouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, data);

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // отвязываем FBO

		// Преобразуем RGB обратно в индекс меша
		unsigned int pickedID = data[0] + (data[1] << 8) + (data[2] << 16) - 1;

		// Проверяем, что индекс валидный
		if (pickedID < loadedModel->getMeshCount())
		{
			loadedModel->selectMesh(pickedID);
			std::cout << "Selected mesh: " << pickedID << std::endl;
		}
	}
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


