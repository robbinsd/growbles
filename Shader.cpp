/*
 *  Shader.cpp
 *  CS248-Final-Project
 *
 *  Created by Matthew Fichman on 1/25/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Shader.h"
#include "Framework.h"
#include <fstream>

#define ERROR_BUFSIZE 1024

Shader::Shader(const std::string& path) :
    path_(path),
    vertexShaderID_(0),
    fragmentShaderID_(0),
    programID_(0),
    loaded_(false)
{
}

void
Shader::Init()
{
    const GLchar* source[1];
    int length = 0;

    // Load the fragment shader and compile
    std::vector<char> fragmentSource = readSource(path_ + ".frag");
    source[0] = &fragmentSource.front();
    length = fragmentSource.size()-1;
    GL_CHECK(fragmentShaderID_ = glCreateShader(GL_FRAGMENT_SHADER));
    GL_CHECK(glShaderSource(fragmentShaderID_, 1, source, &length));
    GL_CHECK(glCompileShader(fragmentShaderID_));

    // Load the vertex shader and compile
    std::vector<char> vertexSource = readSource(path_ + ".vert");
    source[0] = &vertexSource.front();
    length = vertexSource.size()-1;
    GL_CHECK(vertexShaderID_ = glCreateShader(GL_VERTEX_SHADER));
    GL_CHECK(glShaderSource(vertexShaderID_, 1, source, &length));
    GL_CHECK(glCompileShader(vertexShaderID_));

    // Create the vertex program
    programID_ = glCreateProgram();
    GL_CHECK(glAttachShader(programID_, fragmentShaderID_));
    GL_CHECK(glAttachShader(programID_, vertexShaderID_));
    GL_CHECK(glLinkProgram(programID_));

    // Error checking
    GL_CHECK(glGetProgramiv(programID_, GL_LINK_STATUS, (GLint*)&loaded_));
    //glGetShaderiv(vertexShaderID_, GL_COMPILE_STATUS, (GLint*)&loaded_);
    if (!loaded_) {
        GLchar tempErrorLog[ERROR_BUFSIZE];
        GLsizei length;
        glGetShaderInfoLog(fragmentShaderID_, ERROR_BUFSIZE, &length, tempErrorLog);
        errors_ += "Fragment shader errors:\n";
        errors_ += std::string(tempErrorLog, length) + "\n";
        glGetShaderInfoLog(vertexShaderID_, ERROR_BUFSIZE, &length, tempErrorLog);
        errors_ += "Vertex shader errors:\n";
        errors_ += std::string(tempErrorLog, length) + "\n";
        glGetProgramInfoLog(programID_, ERROR_BUFSIZE, &length, tempErrorLog);
        errors_ += "Linker errors:\n";
        errors_ += std::string(tempErrorLog, length) + "\n";
    }
}

Shader::~Shader() {
    GL_CHECK(glDeleteShader(vertexShaderID_));
    GL_CHECK(glDeleteShader(fragmentShaderID_));
    GL_CHECK(glDeleteProgram(programID_));
}

std::vector<char> Shader::readSource(const std::string& path) {

    // Open the file
    std::vector<char> source;
    std::ifstream in(path.c_str());
    if (in.fail()) {
        source.push_back(0);
        return source;
    }

    // Seek to the end of the file to get the size
    in.seekg(0, std::ios::end);
    source.reserve((unsigned)(1 + in.tellg()));
    source.resize((unsigned)in.tellg());
    in.seekg(0, std::ios::beg);
    if (source.empty()) {
        source.push_back(0);
        return source;
    }

    // Now read the whole buffer in one call, and don't 
    // forget to null-terminate the vector with a zero
    in.read(&source.front(), source.size());
    source.push_back(0);

    return source;
}

const std::string& Shader::path() const {
    return path_;
}

GLuint Shader::programID() const {
    return programID_;
}

const std::string& Shader::errors() const {
    return errors_;
}

bool Shader::loaded() const {
    return loaded_;
}
