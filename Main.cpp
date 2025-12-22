#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height); // объявление функции
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"}\n\0";


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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Window_OpenGL_tutorials", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Не удалось создать окно GLFW" << std::endl;
		glfwTerminate(); // завершаем библеотеку
		return -1;
	}

	glfwMakeContextCurrent(window); // все следующие команды, которые будут вызваны относятся к этому окну.
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// получаем и сохраняем адреса функций OpenGL в памяти.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Не удалось инициализировать GLAD" << std::endl;
		return -1;
	}

	unsigned int vertexShader; // сохраняем вершинный шейдер как unsigned int
	vertexShader = glCreateShader(GL_VERTEX_SHADER); //  создаем шейдер с помощью glCreateShader
	// в качестве аргумента указываем тип шейдера, который хотим создать

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); // прикрепляем исходный код шейдера к объекту шейдера
	// 1 аргумент - объект шейдера
	// 2 аргумент - сколько строк мы передаем в качестве исходного кода
	// 3 аргумент - фактический исходный код вершинного шейдера
	// 4 аргумент пока что оставляем пустым
	glCompileShader(vertexShader); // компилируем шейдер

	int success; // определяем целое число, обозначающее успешное выполнение
	char infoLog[512]; // определяем контейнер для хранения ошибок
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); // проверяем успешно ли прошла компиляция
	if (!success) // если компиляция не удалась
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl; // получаем сообщение об ошибке
	}

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// связываем шейдеры
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram(); // создаем программу 
	glAttachShader(shaderProgram, vertexShader); // подключаем шейдер вершин
	glAttachShader(shaderProgram, fragmentShader); // подключаем фрагментный шейдер
	glLinkProgram(shaderProgram); // связываем их с программой с помощью glLinkProgram
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	float vertices[] = {
		// первый треугольник
		0.5f,  0.5f, 0.0f, // правый верхний
	    0.5f, -0.5f, 0.0f,  // правый нижний
	   -0.5f, -0.5f, 0.0f, // левый нижний
	   -0.5f,  0.5f, 0.0f, // нижний левый
	};

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	unsigned int VBO; // объявляем VBO
	unsigned int VAO; // объявляем VAO
	unsigned int EBO; // объявляем буфер элеметов

	glGenBuffers(1, &VBO); // создаем буферный объект с идентификатором 1
	glGenVertexArrays(1, &VAO); // создаем массив вершин
	glGenBuffers(1, &EBO); 

	glBindVertexArray(VAO); // привязываем VAO

	glBindBuffer(GL_ARRAY_BUFFER, VBO); // делаем VBO текущим буфером для цели GL_ARRAY_BUFFER
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // функция предназначенная для копирования пользовательских данных в связанный в данный момент буфер
	/* 1 аргумент функции - это тип буфера, в который мы хотим скопировать данные
	*  2 аргумент указывает размер данных в байтах
	*  3 аргумент фактические данные, которые мы хотим отправить
	*  4 аргумент способ обработки заданных данных */

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); 
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // хранение данных в VBO
	// 1 параметр - какой вершинный атрибут мы хотим настроить
	// 2 параметр - размер вершинного атрибута
	// 3 параметр - тип данных
	// 4 параметр - нормализация данных,  Если мы вводим целочисленные типы данных (int, byte) и устанавливаем для этого параметра значение GL_TRUE, целочисленные данные нормализуются до 0 (или -1 для знаковых данных)
	// и 1 при преобразовании в число с плавающей запятой. Для нас это неактуально, поэтому оставим значение GL_FALSE.
	// 5 параметр - шаг, сообщает нам расстояние между последовательными атрибутами вершин.
	// 6 параметр - смещение, где в буфере начинаются данные о местоположении.
	glEnableVertexAttribArray(0); // активируем конкретный вершинный атрибут с индексом 0 (получается нашу 1 вершину)

	glBindBuffer(GL_ARRAY_BUFFER, 0); 

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window)) // проверяем, нужно ли окну закрыться
	{
		processInput(window); // вызываем процесс ввода на каждой итерации цикла рендеринга

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // очищаем экран выбрав нужный цвет
		glClear(GL_COLOR_BUFFER_BIT); // очищаем только цветной буфер

		// Включаем каркасный режим
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glUseProgram(shaderProgram); // каждый шейдер и вызов рендеринга теперь будет использовать shaderProgram 
		glBindVertexArray(VAO); // как только мы захотим нарисовать объект, мы просто привязываем VAO  к предпочтительным настройкам перед рисованием объекта
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // рисуем 2 треугольника
		// 1 аргумент - тип примитива
		// 2 аргумент - кол-во элементов
		// 3 аргумент - тип индексов, который имеет вид GL_UNSIGNED_INT
		// 4 аргумент - смещение в EBO
		glfwSwapBuffers(window); // меняет местами цветовой буфер
		glfwPollEvents(); // проверка события, например ввод с клавиатуры
	}
	
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteProgram(shaderProgram);

	glfwTerminate(); // удаляем все выделенные ресурсы
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // проверяем нажал ли пользователь Esc
		glfwSetWindowShouldClose(window, true); // если нажал, то мы закрываем GLFW
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) // функция для изменения размера окна экрана 
{
	glViewport(0, 0, width, height); // задаем область просмотра 0,0 - начальная точка, width, height - конечная
}