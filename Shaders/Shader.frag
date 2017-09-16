// Shader.frag
// Created by Elliot Eckholm
// Copyright 2016. BSD License.

#version 330 core

out vec4 color;

uniform vec4 ourColor;

void main()
{
    color = ourColor;
} 