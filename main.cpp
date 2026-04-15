#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include <random>

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

GLuint compilarShader(GLenum tipo, const char* source) {
    GLuint shader = glCreateShader(tipo);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cerr << "Error compilando shader:\n" << infoLog << endl;
    }

    return shader;
}

GLuint crearShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 uColor;
        void main() {
            FragColor = vec4(uColor, 1.0);
        }
    )";

    GLuint vertexShader = compilarShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compilarShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        cerr << "Error enlazando shader program:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

class Grafica{
    public:

    vector<float> construirVerticesGrafica(const vector<double>& datos) {
        vector<float> vertices;

        int n = (int)datos.size();
        if (n == 0) return vertices;
        if (n == 1) {
            float x = -0.9f;
            float y = -0.9f + 1.8f * (float)datos[0];
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            return vertices;
        }

        for (int i = 0; i < n; i++) {
            float x = -0.9f + 1.8f * ((float)i / (float)(n - 1));
            float y = -0.9f + 1.8f * (float)datos[i];

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }

        return vertices;
    }
    vector<float> construirVerticesGraficaAutomatico(const vector<double>& datos) {
        vector<float> vertices;
        int n = (int)datos.size();
        if (n == 0) return vertices;

        double minDato = datos[0];
        double maxDato = datos[0];

        for (double v : datos) {
            if (v < minDato) minDato = v;
            if (v > maxDato) maxDato = v;
        }

        // evitar división por cero
        if (fabs(maxDato - minDato) < 1e-12) {
            maxDato = minDato + 1.0;
        }

        for (int i = 0; i < n; i++) {
            float x = -0.9f + 1.8f * ((float)i / (float)(n - 1));

            float y = -0.9f + 1.8f * (float)((datos[i] - minDato) / (maxDato - minDato));

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(0.0f);
        }

        return vertices;
    }

    GLuint crearVAO(const vector<float>& vertices, GLuint &VBO) {
        GLuint VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return VAO;
    }
    int draw(vector<double> mejoresfitness, vector<double> promedios){
        vector<float> verticesEjes = {
            -0.9f, -0.9f, 0.0f,   0.9f, -0.9f, 0.0f,   // eje X
            -0.9f, -0.9f, 0.0f,  -0.9f,  0.9f, 0.0f    // eje Y
        };
        vector<float> verticesMejor, verticesProm;

        //verticesMejor = construirVerticesGrafica(mejoresfitness);
        //verticesProm  = construirVerticesGrafica(promedios);

        verticesMejor = construirVerticesGraficaAutomatico(mejoresfitness);
        verticesProm  = construirVerticesGraficaAutomatico(promedios);

        

        // 2. Inicializar GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(1000, 700, "Algoritmo Genetico Generaciones vs Fitness", nullptr, nullptr);
        if (window == nullptr) {
            cerr << "No se pudo crear la ventana GLFW" << endl;
            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // 3. Inicializar GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            cerr << "No se pudo inicializar GLAD" << endl;
            return -1;
        }

        // 4. Shaders
        GLuint shaderProgram = crearShaderProgram();
        GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

        // 5. Crear VAO/VBO
        GLuint VBOEjes, VBOMejor, VBOProm;
        GLuint VAOEjes  = crearVAO(verticesEjes, VBOEjes);
        GLuint VAOMejor = crearVAO(verticesMejor, VBOMejor);
        GLuint VAOProm  = crearVAO(verticesProm, VBOProm);

        // 6. Render loop
        while (!glfwWindowShouldClose(window)) {
            processInput(window);

            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(shaderProgram);

            // Ejes blancos
            glBindVertexArray(VAOEjes);
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glDrawArrays(GL_LINES, 0, 4);

            // Mejor fitness - rojo
            if (!verticesMejor.empty()) {
                glBindVertexArray(VAOMejor);
                glUniform3f(colorLoc, 1.0f, 0.0f, 0.0f);
                glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(verticesMejor.size() / 3));
            }

            // Promedio fitness - verde
            if (!verticesProm.empty()) {
                glBindVertexArray(VAOProm);
                glUniform3f(colorLoc, 0.0f, 1.0f, 0.0f);
                glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)(verticesProm.size() / 3));
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
            
        }

        // 7. Liberar recursos
        glDeleteVertexArrays(1, &VAOEjes);
        glDeleteVertexArrays(1, &VAOMejor);
        glDeleteVertexArrays(1, &VAOProm);

        glDeleteBuffers(1, &VBOEjes);
        glDeleteBuffers(1, &VBOMejor);
        glDeleteBuffers(1, &VBOProm);

        glDeleteProgram(shaderProgram);

        glfwTerminate();
        return 0;
    }

};

struct Individuo {
    double x, y;
    double f;
    double fitness;
};
random_device rd;
mt19937 gen(rd());
class AGenetico{
    public:
    double limiteX[2];
    double limiteY[2];
    vector<Individuo> poblacion;
    vector<double> mejoresFitness;
    vector<double> mejoresF;
    vector<double> promediosF;
    vector<double> promediosFitness;
    int tamPoblacion = 100;
    int maxGeneraciones = 100;
    int sinMejoraLimite = 10;
    double mejorFitnessGlobal = -1.0;
    int generacionesSinMejora = 0;
    int promediofitness = 0;
    vector<double> getF(){
        return mejoresF;
    }
    vector<double> getFitness(){
        return mejoresFitness;
    }
    vector<double> getPromedio(){
        return promediosFitness;
    }
    vector<double> getPromedioF(){
        return promediosF;
    }
    void setLimiteX(double x1, double x2){
        limiteX[0] = x1;
        limiteX[1] = x2;
    }
    void setLimiteY(double x1, double x2){
        limiteY[0] = x1;
        limiteY[1] = x2;
    }
    static void evaluar(Individuo &ind) {
        //ind.f = (ind.x * ind.x) - (2 * ind.x * ind.y) + (ind.y * ind.y);
        ind.f = (ind.x * ind.x) - (2 * ind.x * ind.y) + (ind.y * ind.y);
        //ind.fitness = 1.0 / (1.0 + ind.f);
        ind.fitness = 1.0 / (1.0 + ind.f);
        //ind.fitness = ind.f;
        ind.f = ind.f;
    }
    double randomRange(double a, double b) {
        //return a + rand() % (b - a + 1);
        uniform_real_distribution<double> dist(a, b);
        return dist(gen);
    }
    int randomInt(int a, int b) {
        uniform_int_distribution<int> dist(a, b);
        return dist(gen);
    }
    void inicializarPoblacion() {
        poblacion.clear();
        poblacion.resize(tamPoblacion);
        for (auto &ind : poblacion) {
            ind.x = randomRange(limiteX[0], limiteX[1]);
            ind.y = randomRange(limiteY[0], limiteY[1]);
            ind.f = 0;
            ind.fitness = 0.0;
            //cout<<ind.x<<","<<ind.y<<endl;
        }
    }
    double obtenerPromedioFitness() {
        double suma = 0.0;
        for (const auto &ind : poblacion) {
            suma += ind.fitness;
            //suma += ind.f;
        }
        return suma / tamPoblacion;
    }
    double obtenerPromedioF() {
        double suma = 0.0;
        for (const auto &ind : poblacion) {
            suma += ind.f;
            //cout<<ind.f<<endl;
            //suma += ind.f;
        }
        //cout<<"promedio"<<suma / tamPoblacion<<endl;
        return suma / tamPoblacion;
    }
     void evaluarPoblacion() {
        vector<thread> threads;
        threads.reserve(tamPoblacion);
        for (int i = 0; i < tamPoblacion; i++) {
            threads.emplace_back(AGenetico::evaluar, ref(poblacion[i]));
        }
        for (auto &t : threads) {
            t.join();
        }
    }
    Individuo torneo(int k = 3) {
        Individuo mejor = poblacion[randomInt(0, tamPoblacion - 1)];
        for (int i = 1; i < k; i++) {
            Individuo candidato = poblacion[randomInt(0, tamPoblacion - 1)];
            if (candidato.fitness > mejor.fitness) {
                mejor = candidato;
            }
        }
        return mejor;
    }
    pair<Individuo, Individuo> cruzar(const Individuo &p1, const Individuo &p2) {
        Individuo h1 = p1;
        Individuo h2 = p2;
        // cruzamiento simple: intercambiar y
        h1.y = p2.y;
        h2.y = p1.y;
        return {h1, h2};
    }
    Individuo obtenerMejor() {
        Individuo mejor = poblacion[0];
        for (const auto &ind : poblacion) {
            if (ind.fitness > mejor.fitness) {
                mejor = ind;
            }
        }
        return mejor;
    }
    void mutar(Individuo &ind, double probMutacion = 0.1) {
        double r = (double)rand() / RAND_MAX;
        if (r < probMutacion) {
            ind.x += randomRange(-1, 1);
            ind.y += randomRange(-1, 1);

            // asegurar límites
            ind.x = max(limiteX[0], min(ind.x, limiteX[1]));
            ind.y = max(limiteY[0], min(ind.y, limiteY[1]));
        }
    }
    void crearNuevaGeneracion() {
        vector<Individuo> nuevaPoblacion;
        nuevaPoblacion.reserve(tamPoblacion);
        // elitismo: guardar el mejor actual
        Individuo mejorActual = obtenerMejor();
        nuevaPoblacion.push_back(mejorActual);

        while ((int)nuevaPoblacion.size() < tamPoblacion) {
            Individuo padre1 = torneo();
            Individuo padre2 = torneo();

            auto hijos = cruzar(padre1, padre2);

            mutar(hijos.first);
            mutar(hijos.second);

            nuevaPoblacion.push_back(hijos.first);
            if ((int)nuevaPoblacion.size() < tamPoblacion) {
                nuevaPoblacion.push_back(hijos.second);
            }
        }

        poblacion = nuevaPoblacion;
    }
    void ejecutar(){
        inicializarPoblacion();
        double mejorFitnessGlobal = -1.0;
        int generacionesSinMejora = 0;
        for (int gen = 0; gen < maxGeneraciones; gen++) {
            //Evaluo la poblacion con threads y almaceno en fitness
            evaluarPoblacion();
            Individuo mejor = obtenerMejor(); // en este caso como esta minimizando es con <
            double promedioFitness = obtenerPromedioFitness();
            double promedioF = obtenerPromedioF();

            //Almaceno para graficar despues
            mejoresF.push_back(mejor.f);
            mejoresFitness.push_back(mejor.fitness);

            promediosFitness.push_back(promedioFitness);
            promediosF.push_back(promedioF);

            //cout<<"#"<<gen<<"|fitness:"<<mejor.fitness<<"|promedioFitness:"<<promedioFitness<<"("<<mejor.x<<", "<<mejor.y<<")="<<mejor.f<<endl;
            cout<<"#"<<gen<<"|f:"<<mejor.f<<"|promedio:"<<promedioF<<"("<<mejor.x<<", "<<mejor.y<<")="<<mejor.f<<endl;
            
            if (mejor.fitness > mejorFitnessGlobal) {
                mejorFitnessGlobal = mejor.fitness;
                generacionesSinMejora = 0;
            } else {
                generacionesSinMejora++;
            }

            if (generacionesSinMejora >= sinMejoraLimite) {
                cout << "No se encontraron mejores soluciones\n";
                break;
            }
            crearNuevaGeneracion();
        }
    }
    void mostrarHistorial() {
        cout << "\n=== Historial para grafica ===\n";
        cout << "Gen\tMejorFitness\tPromedioFitness\n";
        for (size_t i = 0; i < mejoresFitness.size(); i++) {
            cout << i << "\t" << mejoresFitness[i]
                 << "\t" << promediosFitness[i] << endl;
        }
    }
};
int main(){
    AGenetico ag;
    ag.tamPoblacion = 100;
    ag.maxGeneraciones = 1000;
    ag.sinMejoraLimite = 10;
    ag.setLimiteX(0,31);
    ag.setLimiteY(0,63);
    ag.ejecutar();

    Grafica g;
    //g.draw(ag.getFitness(), ag.getPromedio());
    g.draw(ag.getF(), ag.getPromedioF());
    
    return 0;
}