#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath)
	{
		// 1. Создаём строки, в которые будем считывать исходный код шейдеров
		std::string vertexCode;
		std::string fragmentCode;

		// 2. Создаём файловые потоки для вершинного и фрагментного шейдеров
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		// 3. Настраиваем потоки так, чтобы они выбрасывали исключения при ошибках:
		// например, если файл не найден или произошла ошибка чтения
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			// 4. Открываем файлы шейдеров
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);

			// 5. Создаем строковые потоки для считывания содержимого файлов
			std::stringstream vShaderStream, fShaderStream;

			// 6. Читаем содержимое файлов в строковые потоки
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			// 7. Закрываем файловые потоки - особождаем ресурсы
			vShaderFile.close();
			fShaderFile.close();

			// 8. Конвертируем содержимое потоков в строки
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}

		catch (std::ifstream::failure e)
		{
			// 9. Если произошла ошибка при открытии или чтении файлов, выводим сообщение
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}

		// 10. Получаем C-style строки для передачи в OpenGL
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		// Компиляция шейдеров
		unsigned int vertex, fragment; // идентификация шейдеров
		int success; // флаг успешной компиляции
		char infoLog[512]; // буфер для хранения сообщения об ошибках

		// Вершинный шейдер 

		vertex = glCreateShader(GL_VERTEX_SHADER); // создаем объект вершинного шейдера
		glShaderSource(vertex, 1, &vShaderCode, NULL); // прикрепляем исходный код
		glCompileShader(vertex); // компилируем шейдер

		// проверяем успешность компиляции
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			// если компиляция не удалась, то получаем лог ошибок
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Фрагментный шейдер

		fragment = glCreateShader(GL_FRAGMENT_SHADER); // создаем объект фрагментного шнейдера
		glShaderSource(fragment, 1, &fShaderCode, NULL); // прикрепляем исходный код
		glCompileShader(fragment); // компилируем шейдер

		// проверяем успешность компиляции фрагментного шейдера
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Создание программы шейдеров 

		ID = glCreateProgram(); // создаем объект программы
		glAttachShader(ID, vertex); // прикрепляем вершинный шейдер
		glAttachShader(ID, fragment); // прикрепляем фрагментный шейдер
		glLinkProgram(ID); // линковка шейдерной программы

		// проверяем успешность линковки программы
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		// шейдеры уже объединены в программу, их можно удалить
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	// Активация шейдера 
	// Эта функция активирует шейдерную программу, чтобы OpenGL использовал её при отрисовке
	void use()
	{
		glUseProgram(ID); // OpenGL начинает использовать шейдер с идентификатором ID
	}

	// Утилитарные функции для uniform
	// Эти функции помогают "передавать" значения из С++ в шейдеры

	// 1. Установка bool-переменной в шейдере

	void setBool(const std::string& name, bool value) const
	{
		// glGetUniformLocation(ID, name.c_str()) возвращает "адрес" uniform-переменной
		// glUniform1i устанавливает целочисленное значение (bool хранится как 0 или 1)
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}

	// 2. Установка int-перемнной в шейдере
	void setInt(const std::string& name, int value) const
	{
		// glUniform1i устанавливает целое число для uniform-переменной
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}

	// 3. Установка float-переменной в шейдере
	void setFloat(const std::string& name, float value) const
	{
		// glUniform1f устанавливает число с плавающей точкой для uniform-переменной
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	// 4. Установка setMat4 
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
	}

	//5. Установка vec3-переменной
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
};

#endif

