#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <SOIL.h>

#define _USE_MATH_DEFINES // для использования математических констант
#include <math.h>

const GLuint count = 5;

glm::vec3 shifts[count] = { glm::vec3(0.0, 0.0, 0.0),
                            glm::vec3(0.0, 20.0, -40.0),
                            glm::vec3(0.0, -100.0, 0.0),
                            glm::vec3(0.0, 5.0, 18.0),
                            glm::vec3(-9.0, 1.0, 20.0),
 };

// Источник освещения
struct Light {
    GLfloat inten; // Интенсивность (от 0.0 до 1.0)
    glm::vec3 pos; // Позиция источника
    glm::vec3 dir; // Направление источника (точка, куда он смотрит)
    
    Light(GLfloat intensity, glm::vec3 position, glm::vec3 direction)
    {
        pos = position;
        inten = intensity;
        dir = direction;
    }
};

// Структура для представления камеры в 3D пространстве
struct Camera {
    glm::vec3 position; // Позиция камеры в мировых координатах
    glm::vec3 front;    // Направление взгляда камеры
    glm::vec3 up;       // Вектор "вверх" для камеры
    float yaw;          // Угол поворота по горизонтали
    float pitch;        // Угол поворота по вертикали
    float movementSpeed; // Скорость движения камеры

    // Конструктор камеры, инициализирует позицию и векторы
    Camera(glm::vec3 startPos)
        : position(startPos), front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)),
        yaw(90.0f), pitch(-30.0f), movementSpeed(0.5f) {
        updateCameraVectors(); // Обновление векторов камеры
    }

    // Функция для обновления направления взгляда и вектора "вверх"
    void updateCameraVectors() {
        glm::vec3 front; // Вектор направления взгляда
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); // Вычисление направления по оси X
        front.y = sin(glm::radians(pitch)); // Вычисление направления по оси Y
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)); // Вычисление направления по оси Z
        this->front = glm::normalize(front); // Нормализация вектора направления взгляда
        glm::vec3 right = glm::normalize(glm::cross(this->front, glm::vec3(0.0f, 1.0f, 0.0f))); // Вычисление вектора "право"
        this->up = glm::normalize(glm::cross(right, this->front)); // Пересчет вектора "вверх"
    }

    // Функция для обработки ввода с клавиатуры и перемещения камеры
    void moveCamera(sf::Keyboard::Key key, float deltaTime) {
        float velocity = movementSpeed * deltaTime; // Вычисление скорости перемещения
        if (key == sf::Keyboard::W)
            position += front * velocity; // Движение вперёд
        if (key == sf::Keyboard::S)
            position -= front * velocity; // Движение назад
        if (key == sf::Keyboard::A)
            position -= glm::normalize(glm::cross(front, up)) * velocity; // Движение влево
        if (key == sf::Keyboard::D)
            position += glm::normalize(glm::cross(front, up)) * velocity; // Движение вправо
        if (key == sf::Keyboard::R)
            position += up * velocity; // Движение вверх
        if (key == sf::Keyboard::F)
            position -= up * velocity; // Движение вниз
        updateCameraVectors(); // Обновление векторов камеры
    }

    // Функция для вращения камеры
    void rotateCamera(float yawOffset, float pitchOffset) {
        yaw += yawOffset; // Обновление угла поворота по горизонтали
        pitch += pitchOffset; // Обновление угла поворота по вертикали

        if (pitch < 0.0f)
            pitch += 360.0f;
        if (pitch >= 360.0f)
            pitch -= 360.0f;

        // Обеспечение 360-градусного поворота по yaw
        if (yaw < 0.0f)
            yaw += 360.0f;
        if (yaw >= 360.0f)
            yaw -= 360.0f;

        updateCameraVectors();
    }

    // Функция для получения матрицы вида
    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }
};

// Фигуры
std::vector<std::vector<GLfloat>> figures;

std::string readAllFile(const std::string filename) {
    std::fstream fs(filename);
    std::ostringstream sstr;
    sstr << fs.rdbuf();
    return sstr.str();
}

void loadOBJ(const std::string filename) {
    std::vector<GLfloat> verts; // Вершины
    std::vector<GLfloat> texs; // Текстурные координаты
    std::vector<GLfloat> normals; // Нормали
    std::vector<GLint> indVerts; // Индексы вершин
    std::vector<GLint> indTexs; // Индексы текстур
    std::vector<GLint> indNorms; // Индексы нормалей

    std::fstream obj(filename);
    if (!obj.is_open()) {
        std::cerr << "Не удалось загрузить obj файл" << std::endl;
        return;
    }

    std::string line;
    std::string type;

    while (std::getline(obj, line)) {
        std::stringstream sstr;
        sstr << line;
        sstr >> type;

        if (type == "v") { // Если строка начинается с 'v' (вершина)
            GLfloat x, y, z;
            sstr >> x >> y >> z;
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
        }
        else if (type == "vn") { // Если строка начинается с 'vn' (нормаль)
            GLfloat nx, ny, nz;
            sstr >> nx >> ny >> nz;
            normals.push_back(nx);
            normals.push_back(ny);
            normals.push_back(nz);
        }
        else if (type == "vt") { // Если строка начинается с 'vt' (текстурная координата)
            GLfloat numb;
            sstr >> numb; // Чтение текстурной координаты U
            texs.push_back(numb);
            sstr >> numb; // Чтение текстурной координаты V
            texs.push_back(numb);
        }
        else if (type == "f") { // Если строка начинается с 'f' (фигура)
            GLint intNumb;

            while (sstr >> intNumb) { // Чтение индекса вершины
                indVerts.push_back(intNumb - 1); // Сохранение индекса вершины (с учетом смещения)
                char divider;
                sstr >> divider;

                if (sstr.peek() == '/') // Если текстурный индекс отсутствует
                    indTexs.push_back(-1); // Отметка отсутствия текстурного индекса
                else
                {
                    sstr >> intNumb;
                    indTexs.push_back(intNumb - 1); // Сохранение текстурного индекса
                }

                sstr >> divider >> intNumb;
                indNorms.push_back(intNumb - 1); // Сохранение индекса нормали
            }
        }
    }

    // Создание фигуры на основе загруженных данных
    std::vector<GLfloat> figure;
    for (int i = 0; i < indVerts.size(); ++i) {
        // Добавление координат вершин
        for (int j = 0; j < 3; ++j)
            figure.push_back(verts[3 * indVerts[i] + j]);

        // Добавление нормалей
        for (int j = 0; j < 3; ++j)
            figure.push_back(normals[3 * indNorms[i] + j]);

        // Добавление текстурных координат
        if (indTexs[i] != -1) {
            figure.push_back(texs[2 * indTexs[i]]);
            figure.push_back(1.0 - texs[2 * indTexs[i] + 1]);
        }
        else {
            figure.push_back(0); // Если текстурные координаты отсутствуют, добавляем 0
            figure.push_back(0);
        }
    }

    figures.push_back(figure);
}

GLuint compileShader(const GLchar* source, GLenum type) {
    GLuint shader = glCreateShader(type); // Создание шейдера
    glShaderSource(shader, 1, &source, nullptr); // Привязка исходного кода шейдера
    glCompileShader(shader); // Компиляция шейдера

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader; // Возврат ID скомпилированного шейдера
}

GLuint createShaderProgram(const std::string vShader, const std::string frShader) {
    std::string vertexShaderSource = readAllFile(vShader);
    std::string fragmentShaderSource = readAllFile(frShader);

    GLuint vertexShader = compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint link_ok;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &link_ok);

    if (!link_ok) {
        std::cout << "Error attach shaders \n";
    }

    return shaderProgram;
}


// Функция для создания фигуры (VAO и VBO)
void createShape(GLuint& VBO, GLuint& VAO, GLuint ind) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Передача данных вершин в VBO
    glBufferData(GL_ARRAY_BUFFER, figures[ind].size() * sizeof(GLfloat), figures[ind].data(), GL_STATIC_DRAW);

    // Установка указателей для атрибутов вершин
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // Позиции
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat))); // Нормали
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat))); // Текстурные координаты
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glBindVertexArray(0);
}

int main() {
    setlocale(LC_ALL, "Russian");
    sf::ContextSettings settings;
    settings.depthBits = 24;
    sf::Window window(sf::VideoMode(800, 600), "Window", sf::Style::Default, settings);

    glewInit();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    GLboolean flag = GL_FALSE;

    GLuint vboObj[count]; // Массив переменных для хранения VBO 
    GLuint vaoObj[count]; // Массив переменных для хранения VAO
    GLuint texObj[count]; // Массив переменных для хранения текстуры

    glEnable(GL_TEXTURE_2D); // Включение текстурирования

    // Создание шейдерной программы для планет
    GLuint shaderObjectProgram[count] = { createShaderProgram("field_vertex_shader.glsl", "field_fragment_shader.glsl"),
                                          createShaderProgram("airship_vertex_shader.glsl", "airship_fragment_shader.glsl"),
                                          createShaderProgram("gift_vertex_shader.glsl", "gift_fragment_shader.glsl"),
                                          createShaderProgram("target_vertex_shader.glsl", "target_fragment_shader.glsl"),
                                          createShaderProgram("tree_vertex_shader.glsl", "tree_fragment_shader.glsl"),
    };

    // Загрузка моделей
    loadOBJ("objects/mountain.obj");
    loadOBJ("objects/wolf.obj");
    loadOBJ("objects/gift.obj");
    loadOBJ("objects/target.obj");
    loadOBJ("objects/tree.obj");

    // Создание VAO и VBO
    for (int i = 0; i < count; ++i)
        createShape(vboObj[i], vaoObj[i], i);

    GLfloat scale = 1.0;

    GLfloat floorGift = 0.0;

    GLuint* tex = &texObj[0];
    int q = 0;
    for (std::string filename : {"objects/mountain.jpeg", "objects/wolf.png", "objects/gift.png", "objects/tree.jpeg"})
    {
        int weight, height;
        unsigned char* image = SOIL_load_image(filename.c_str(), &weight, &height, 0, SOIL_LOAD_RGB);
        glGenTextures(1, tex);
        glBindTexture(GL_TEXTURE_2D, *tex);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Установка параметров текстурирования
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, weight, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image); // Передача данных текстуры

        SOIL_free_image_data(image);
        ++q;
        tex = &texObj[q];
    }

    Camera camera(glm::vec3(0.0f, 25.0f, -45.0f)); // Начальная позиция камеры

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            camera.moveCamera(sf::Keyboard::W, 0.1f); // движение вперёд
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            camera.moveCamera(sf::Keyboard::S, 0.1f); // движение назад
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            camera.moveCamera(sf::Keyboard::A, 0.1f); // движение влево
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            camera.moveCamera(sf::Keyboard::D, 0.1f); // движение вправо
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            camera.moveCamera(sf::Keyboard::R, 0.1f); // движение вверх
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::F))
            camera.moveCamera(sf::Keyboard::F, 0.1f); // движение вниз

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) // для горизонтального поворота
            camera.rotateCamera(0.1f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            camera.rotateCamera(-0.1f, 0.0f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) // для вертикального поворота
            camera.rotateCamera(0.0f, 0.1f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            camera.rotateCamera(0.0f, -0.1f);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
            if (shifts[2].y == -100)
            {
                shifts[2] = shifts[1];
                shifts[2].x += 1.3;
                shifts[2].y -= 2.3;
                shifts[2].z += 1.0;
                flag = GL_TRUE;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Обновление матрицы вида
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 mvp = projection * view;

        if (shifts[1].z < 45)
            shifts[1].z += 0.01;
        else
            shifts[1].z = -40.0;

        if (flag)
        {
            if (shifts[2].z <= 22 && shifts[2].z > 7)
                floorGift = 5;
            else if (shifts[2].z <= 7 && shifts[2].z > -10)
                floorGift = 8;
            else if (shifts[2].z <= -10 && shifts[2].z > -51)
                floorGift = 12.5;
            else if (shifts[2].z <= -51)
                floorGift = 14;

            if (shifts[2].y > floorGift)
                shifts[2].y -= 0.01;
            else
            {
                shifts[2] = glm::vec3(0.0, -100.0, 0.0);
                flag = GL_FALSE;
            }
            
            if ((abs(shifts[2].x - shifts[3].x) <= 3) && ((shifts[2].y - 2) <= shifts[3].y) && (abs(shifts[2].z - shifts[3].z) <= 3))
            {
                std::cerr << "ГООООООЛ!" << std::endl;
                shifts[3].z = rand() % 61 - 30;
                if (shifts[3].z <= 22 && shifts[3].z > 7)
                    shifts[3].y = 5;
                else if (shifts[3].z <= 7 && shifts[3].z > -10)
                    shifts[3].y = 8;
                else if (shifts[3].z <= -10 && shifts[3].z > -51)
                    shifts[3].y = 12.5;
                else if (shifts[3].z <= -51)
                    shifts[3].y = 14;
                shifts[2] = glm::vec3(0.0, -100.0, 0.0);
                flag = GL_FALSE;
            }
        }

        if (scale > 0.4)
            scale -= 0.0008;
        else
            scale = 1.0;

        for (int i = 0; i < count; ++i)
        {
            glUseProgram(shaderObjectProgram[i]);
            glUniformMatrix4fv(glGetUniformLocation(shaderObjectProgram[i], "matr"), 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3fv(glGetUniformLocation(shaderObjectProgram[i], "shift"), 1, glm::value_ptr(shifts[i]));
            glUniform3f(glGetUniformLocation(shaderObjectProgram[i], "posView"), camera.position[0], camera.position[1], camera.position[2]);

            if (i == 3)
                glUniform1f(glGetUniformLocation(shaderObjectProgram[i], "scale"), scale);
            else
            {
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "lightPosition"), 60.0f, 60.0f, -60.0f, 1.0);
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "lightAmbient"), 0.1, 0.1, 0.1, 1.0);
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "lightDiffuse"), 0.8, 0.8, 0.8, 1.0);
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "lightSpecular"), 0.7, 0.7, 0.7, 1.0);

                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "materialAmbient"), 0.1, 0.1, 0.1, 1);
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "materialDiffuse"), 0.4, 0.4, 0.4, 1);
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "materialSpecular"), 0.4, 0.4, 0.4, 1);
                glUniform4f(glGetUniformLocation(shaderObjectProgram[i], "materialEmission"), 0.4, 0.4, 0.4, 1);
                glUniform1f(glGetUniformLocation(shaderObjectProgram[i], "materialShininess"), 0.8);
            }
            glBindVertexArray(vaoObj[i]);
            if ( i != 4 && (i != 2 || flag))
                glBindTexture(GL_TEXTURE_2D, texObj[i]);
            if (i == 4)
                glBindTexture(GL_TEXTURE_2D, texObj[i - 1]);
            if (i != 3)
                glDrawArrays(GL_TRIANGLES, 0, figures[i].size() / 8);
            else
            {
                glBindTexture(GL_TEXTURE_2D, NULL);
                glDrawArrays(GL_QUADS, 0, figures[i].size() / 8);
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }

        window.display();
    }

    for (int i = 0; i < count; ++i)
    {
        glDeleteBuffers(1, &vboObj[i]);
        glDeleteBuffers(1, &texObj[i]);
        glDeleteVertexArrays(1, &vaoObj[i]);
        glDeleteProgram(shaderObjectProgram[i]);
    }

    return 0;
}