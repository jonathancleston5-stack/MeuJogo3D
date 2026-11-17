#include <cstring>
#include <algorithm>
#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <cstdio>

const char* caminhoConfig() {
#ifdef PLATFORM_ANDROID
    return "/data/data/com.jonathan.meujogo3d/files/config.txt";
#else
    return "config.txt";
#endif
}

void salvarConfig(bool lanterna, int fps, bool som, bool showFps, int nivelRes) {
    FILE* f = std::fopen(caminhoConfig(), "w");
    if (f) {
        std::fprintf(f, "%d %d %d %d %d\n", lanterna ? 1 : 0, fps, som ? 1 : 0, showFps ? 1 : 0, nivelRes);
        std::fclose(f);
    }
}

void carregarConfig(bool &lanterna, int &fps, bool &som, bool &showFps, int &nivelRes) {
    FILE* f = std::fopen(caminhoConfig(), "r");
    if (f) {
        int l = 1, fp = 30, s = 1, sf = 1, nr = 0;
        if (std::fscanf(f, "%d %d %d %d %d", &l, &fp, &s, &sf, &nr) == 5) {
            lanterna = (l != 0); fps = (fp == 60) ? 60 : 30;
            som = (s != 0); showFps = (sf != 0);
            nivelRes = (nr >= 0 && nr <= 2) ? nr : 0;
        }
        std::fclose(f);
    }
}

Shader shaderConsole;
Model modHarry, modLanternaPSX;
float escalaHarry = 1.0f, offsetHarryY = 0.0f, offsetHarryX = 0.0f, offsetHarryZ = 0.0f;
float escalaLanternaPSX = 1.0f;
bool harryPronto = false;
ModelAnimation *animsHarry = nullptr;
int animCountHarry = 0;
int animFrameHarry = 0;
int idxWalk = 0, idxIdle = 0;
float timerAnimHarry = 0.0f;
Model modPoste, modLanterna;
float escalaPoste = 1.0f, escalaLanterna = 1.0f;
bool modelosExtrasProntos = false;

void desenharBonecoPS1(Vector3 pos, float angulo, float cicloPasso, bool agachado) {
    if (!harryPronto) {
        modHarry = LoadModel("personagem.gltf");
        BoundingBox bb = GetModelBoundingBox(modHarry);
        float alt = bb.max.y - bb.min.y;
        if (alt > 0.001f) {
            escalaHarry = 1.75f / alt;
            offsetHarryY = -bb.min.y * escalaHarry;
            offsetHarryX = -((bb.min.x + bb.max.x) * 0.5f) * escalaHarry;
            offsetHarryZ = -((bb.min.z + bb.max.z) * 0.5f) * escalaHarry;
        }
        animsHarry = LoadModelAnimations("personagem.gltf", &animCountHarry);
        for (int i = 0; i < animCountHarry; i++) {
            const char* nm = animsHarry[i].name;
            if (std::strstr(nm, "Walk") != nullptr || std::strstr(nm, "walk") != nullptr) idxWalk = i;
            if (std::strstr(nm, "Idle") != nullptr || std::strstr(nm, "idle") != nullptr) idxIdle = i;
        }

        modLanternaPSX = LoadModel("lanterna_psx.glb");
        BoundingBox bbL = GetModelBoundingBox(modLanternaPSX);
        float tamL = std::max({bbL.max.x - bbL.min.x, bbL.max.y - bbL.min.y, bbL.max.z - bbL.min.z});
        if (tamL > 0.001f) escalaLanternaPSX = 0.28f / tamL;

        harryPronto = true;
    }

    bool andando = (cicloPasso > 0.01f);
    if (animCountHarry > 0 && animsHarry != nullptr) {
        int animAtual = andando ? idxWalk : idxIdle;
        if (animAtual >= animCountHarry) animAtual = 0;
        timerAnimHarry += GetFrameTime() * (andando ? (agachado ? 22.0f : 32.0f) : 24.0f);
        if (animsHarry[animAtual].keyframeCount > 0) {
            animFrameHarry = ((int)timerAnimHarry) % animsHarry[animAtual].keyframeCount;
            UpdateModelAnimation(modHarry, animsHarry[animAtual], animFrameHarry);
        }
    }

    float escalaY = agachado ? (escalaHarry * 0.72f) : escalaHarry;

    rlPushMatrix();
        rlTranslatef(pos.x, pos.y, pos.z);
        rlRotatef(angulo, 0.0f, 1.0f, 0.0f);
        if (agachado) rlRotatef(18.0f, 1.0f, 0.0f, 0.0f);

        DrawModelEx(
            modHarry,
            { offsetHarryX, offsetHarryY, offsetHarryZ },
            { 0.0f, 1.0f, 0.0f },
            0.0f,
            { escalaHarry, escalaY, escalaHarry },
            Color{ 165, 170, 185, 255 }
        );

        float altMao = agachado ? 0.64f : 0.88f;
        DrawModelEx(
            modLanternaPSX,
            { 0.24f, altMao, 0.14f },
            { 0.0f, 1.0f, 0.0f },
            0.0f,
            { escalaLanternaPSX, escalaLanternaPSX, escalaLanternaPSX },
            Color{ 200, 200, 200, 255 }
        );
    rlPopMatrix();
}

const char* vsCode =
    "#version 100\n"
    "attribute vec3 vertexPosition;\n"
    "attribute vec2 vertexTexCoord;\n"
    "attribute vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n"
    "varying vec3 fragPos;\n"
    "varying vec2 fragTexCoord;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    fragColor = vertexColor;\n"
    "    gl_Position = mvp * vec4(vertexPosition, 1.0);\n"
    "}\n";

const char* fsCode =
    "#version 100\n"
    "precision mediump float;\n"
    "varying vec2 fragTexCoord;\n"
    "varying vec4 fragColor;\n"
    "varying vec3 fragPos;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec3 lightPos;\n"
    "uniform vec3 lightDir;\n"
    "uniform int lanternaOn;\n"
    "void main() {\n"
    "    vec4 texel = texture2D(texture0, fragTexCoord) * fragColor;\n"
    "    // Noite bem escura para destacar a lanterna\n"
    "    vec3 luzTotal = vec3(0.04, 0.05, 0.08);\n"
    "    if (lanternaOn == 1) {\n"
    "        vec3 toFrag = fragPos - lightPos;\n"
    "        float distLuz = length(toFrag);\n"
    "        float spot = dot(normalize(toFrag), normalize(lightDir));\n"
    "        float cone = smoothstep(0.65, 0.90, spot);\n"
    "        float nucleo = smoothstep(0.86, 0.98, spot);\n"
    "        float atten = clamp(1.0 - distLuz / 28.0, 0.0, 1.0);\n"
    "        atten = atten * atten;\n"
    "        float feixe = clamp((cone * 1.4 + nucleo * 1.3) * atten, 0.0, 2.1);\n"
    "        luzTotal += vec3(1.0, 0.95, 0.80) * feixe;\n"
    "    }\n"
    "    gl_FragColor = vec4(texel.rgb * luzTotal, texel.a);\n"
    "}\n";

Model modCarroBase, modCarroTeto;
bool modCarroPronto = false;

Model modAsfalto, modCalcada, modPredio;
bool modelosRuaProntos = false;


Model modCarSedan, modCarPolice, modCarTaxi, modCarWheel;
Texture2D texCarGrey, texCarGreen, texCarPolice, texCarTaxi, texCarWheel;
float escalaCarroPS1 = 1.0f, offsetCarroY = 0.0f, escalaRodaPS1 = 1.0f;
bool carrosPS1Prontos = false;
int contadorCarroChamada = 0;

void desenharCarroAbandonado(float x, float z, float rotY, Color corLataria) {
    if (!carrosPS1Prontos) {
        modCarSedan  = LoadModel("Car5.obj");
        modCarPolice = LoadModel("Car5_Police.obj");
        modCarTaxi   = LoadModel("Car5_Taxi.obj");
        modCarWheel  = LoadModel("Wheel.obj");

        texCarGrey   = LoadTexture("car5_grey.png");
        texCarGreen  = LoadTexture("car5_green.png");
        texCarPolice = LoadTexture("car5_police.png");
        texCarTaxi   = LoadTexture("car5_taxi.png");
        texCarWheel  = LoadTexture("wheel.png");

        SetTextureFilter(texCarGrey, TEXTURE_FILTER_POINT);
        SetTextureFilter(texCarGreen, TEXTURE_FILTER_POINT);
        SetTextureFilter(texCarPolice, TEXTURE_FILTER_POINT);
        SetTextureFilter(texCarTaxi, TEXTURE_FILTER_POINT);
        SetTextureFilter(texCarWheel, TEXTURE_FILTER_POINT);

        modCarSedan.materials.maps[MATERIAL_MAP_DIFFUSE].texture  = texCarGrey;
        modCarPolice.materials.maps[MATERIAL_MAP_DIFFUSE].texture = texCarPolice;
        modCarTaxi.materials.maps[MATERIAL_MAP_DIFFUSE].texture   = texCarTaxi;
        modCarWheel.materials.maps[MATERIAL_MAP_DIFFUSE].texture  = texCarWheel;

        modCarSedan.materials.shader  = shaderConsole;
        modCarPolice.materials.shader = shaderConsole;
        modCarTaxi.materials.shader   = shaderConsole;
        modCarWheel.materials.shader  = shaderConsole;

        BoundingBox bb = GetModelBoundingBox(modCarPolice);
        float comp = std::max(bb.max.x - bb.min.x, bb.max.z - bb.min.z);
        if (comp > 0.01f) {
            escalaCarroPS1 = 4.35f / comp;
            offsetCarroY   = -bb.min.y * escalaCarroPS1 + 0.18f;
        }
        BoundingBox bbW = GetModelBoundingBox(modCarWheel);
        float diamW = bbW.max.y - bbW.min.y;
        if (diamW > 0.01f) {
            escalaRodaPS1 = 0.64f / diamW;
        }
        carrosPS1Prontos = true;
    }

    int tipo = contadorCarroChamada % 3;
    contadorCarroChamada++;

    rlPushMatrix();
        rlTranslatef(x, 0.0f, z);
        rlRotatef(rotY, 0.0f, 1.0f, 0.0f);

        // Sombra de contato escura embaixo do carro
        DrawCylinder({ 0.0f, 0.02f, 0.0f }, 1.15f, 1.15f, 0.02f, 12, Color{ 5, 6, 8, 190 });

        if (tipo == 0) {
            DrawModelEx(modCarPolice, { 0.0f, offsetCarroY, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f, { escalaCarroPS1, escalaCarroPS1, escalaCarroPS1 }, WHITE);
        } else if (tipo == 1) {
            DrawModelEx(modCarTaxi, { 0.0f, offsetCarroY, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f, { escalaCarroPS1, escalaCarroPS1, escalaCarroPS1 }, WHITE);
        } else {
            modCarSedan.materials.maps[MATERIAL_MAP_DIFFUSE].texture = (corLataria.r > corLataria.g) ? texCarGrey : texCarGreen;
            DrawModelEx(modCarSedan, { 0.0f, offsetCarroY, 0.0f }, { 0.0f, 1.0f, 0.0f }, 0.0f, { escalaCarroPS1, escalaCarroPS1, escalaCarroPS1 }, WHITE);
        }

        // 4 Rodas 3D PS1
        float rx = 0.86f;
        float rz = 1.32f;
        DrawModelEx(modCarWheel, { -rx, 0.32f,  rz }, { 0.0f, 1.0f, 0.0f }, 180.0f, { escalaRodaPS1, escalaRodaPS1, escalaRodaPS1 }, WHITE);
        DrawModelEx(modCarWheel, {  rx, 0.32f,  rz }, { 0.0f, 1.0f, 0.0f },   0.0f, { escalaRodaPS1, escalaRodaPS1, escalaRodaPS1 }, WHITE);
        DrawModelEx(modCarWheel, { -rx, 0.32f, -rz }, { 0.0f, 1.0f, 0.0f }, 180.0f, { escalaRodaPS1, escalaRodaPS1, escalaRodaPS1 }, WHITE);
        DrawModelEx(modCarWheel, {  rx, 0.32f, -rz }, { 0.0f, 1.0f, 0.0f },   0.0f, { escalaRodaPS1, escalaRodaPS1, escalaRodaPS1 }, WHITE);
    rlPopMatrix();
}

void desenharRuaSilentHill() {
    // Muros fechando os becos laterais
    for (float bz = -40.0f; bz <= 40.0f; bz += 20.0f) {
        DrawCube(Vector3{ -14.5f, 2.5f, bz }, 1.0f, 5.0f, 12.0f, Color{ 18, 20, 25, 255 });
        DrawCube(Vector3{  14.5f, 2.5f, bz }, 1.0f, 5.0f, 12.0f, Color{ 18, 20, 25, 255 });
    }

    if (!modelosRuaProntos) {
        // Textura procedural de asfalto rugoso e manchado (sem xadrez)
        Image i1 = GenImagePerlinNoise(128, 128, 12, 34, 6.5f);
        ImageColorTint(&i1, Color{ 75, 80, 88, 255 });
        for (int py = 0; py < 128; py++) {
            for (int px = 0; px < 128; px++) {
                int r = ((px * 73 + py * 137) % 19) - 9;
                Color c = GetImageColor(i1, px, py);
                c.r = (unsigned char)std::clamp(c.r + r, 15, 255);
                c.g = (unsigned char)std::clamp(c.g + r, 15, 255);
                c.b = (unsigned char)std::clamp(c.b + r + 2, 18, 255);
                ImageDrawPixel(&i1, px, py, c);
            }
        }
        modAsfalto = LoadModelFromMesh(GenMeshPlane(12.0f, 80.0f, 1, 1));
        Texture2D texAsf = LoadTextureFromImage(i1);
        SetTextureFilter(texAsf, TEXTURE_FILTER_POINT);
        modAsfalto.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texAsf;
        UnloadImage(i1);

        // Textura procedural de concreto gasto para calcadas e predios
        Image i2 = GenImagePerlinNoise(128, 128, 55, 19, 4.5f);
        ImageColorTint(&i2, Color{ 135, 138, 142, 255 });
        for (int py = 0; py < 128; py++) {
            for (int px = 0; px < 128; px++) {
                int r = ((px * 31 + py * 97) % 15) - 7;
                if (py % 32 == 0) r -= 18; // Juntas de dilatacao do concreto
                Color c = GetImageColor(i2, px, py);
                c.r = (unsigned char)std::clamp(c.r + r, 25, 255);
                c.g = (unsigned char)std::clamp(c.g + r, 25, 255);
                c.b = (unsigned char)std::clamp(c.b + r, 25, 255);
                ImageDrawPixel(&i2, px, py, c);
            }
        }
        modCalcada = LoadModelFromMesh(GenMeshCube(4.4f, 0.24f, 80.0f));
        Texture2D texCalc = LoadTextureFromImage(i2);
        SetTextureFilter(texCalc, TEXTURE_FILTER_POINT);
        modCalcada.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texCalc;
        UnloadImage(i2);
        Image i3 = GenImageChecked(32, 32, 4, 8, Color{72,76,80,255}, Color{60,64,68,255});
        modPredio = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
        Texture2D texParede = LoadTexture("parede.png");
        SetTextureWrap(texParede, TEXTURE_WRAP_REPEAT);
        modPredio.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texParede;
        modPredio.materials[0].shader = shaderConsole;
        modelosRuaProntos = true;
    }

    // Asfalto e faixas
    DrawModel(modAsfalto, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, Color{ 160, 165, 170, 255 });
    for (float z = -36.0f; z <= 36.0f; z += 4.5f)
        DrawCube(Vector3{ 0.0f, 0.015f, z }, 0.22f, 0.02f, 2.2f, Color{ 185, 155, 50, 255 });

    // Calcadas e chao escuro dos becos laterais
    DrawModel(modCalcada, Vector3{ -8.2f, 0.12f, 0.0f }, 1.0f, Color{ 185, 190, 195, 255 });
    DrawModel(modCalcada, Vector3{  8.2f, 0.12f, 0.0f }, 1.0f, Color{ 185, 190, 195, 255 });
    DrawCube(Vector3{ -18.0f, 0.05f, 0.0f }, 16.0f, 0.10f, 80.0f, Color{ 14, 16, 18, 255 });
    DrawCube(Vector3{  18.0f, 0.05f, 0.0f }, 16.0f, 0.10f, 80.0f, Color{ 14, 16, 18, 255 });

    // Predios separados por becos escuros (espacos de 4m a 5m entre eles)
    struct Predio { float x, z, w, h, d; Color cor; };
    Predio ps[] = {
        {-15.2f,-28.0f,9.5f,16.0f,13.0f,{76,80,84,255}}, {-15.2f,-10.0f,9.5f,21.0f,13.5f,{88,82,78,255}},
        {-15.2f,  9.0f,9.5f,14.0f,14.0f,{70,76,82,255}}, {-15.2f, 28.0f,9.5f,19.0f,13.0f,{82,80,76,255}},
        { 15.2f,-27.0f,9.5f,18.0f,13.5f,{80,78,74,255}}, { 15.2f, -8.0f,9.5f,15.0f,13.0f,{68,74,80,255}},
        { 15.2f, 10.0f,9.5f,22.0f,13.5f,{85,80,76,255}}, { 15.2f, 28.0f,9.5f,16.0f,13.0f,{74,78,84,255}}
    };
    for (int i = 0; i < 8; i++) {
        DrawModelEx(modPredio, Vector3{ ps[i].x, ps[i].h/2.0f, ps[i].z }, Vector3{0,1,0}, 0.0f, Vector3{ ps[i].w, ps[i].h, ps[i].d }, ps[i].cor);
        float fx = (ps[i].x < 0) ? (ps[i].x + ps[i].w/2.0f + 0.04f) : (ps[i].x - ps[i].w/2.0f - 0.04f);
        int idxJ = 0;
        for (float wy = 3.5f; wy < ps[i].h - 2.0f; wy += 3.2f) {
            for (float wz = ps[i].z - ps[i].d/2.0f + 2.2f; wz <= ps[i].z + ps[i].d/2.0f - 2.2f; wz += 3.2f) {
                idxJ++;
                // Vidros quebrados: deixa estilhacos pontudos nas bordas em varias janelas
                DrawCube(Vector3{ fx, wy, wz }, 0.08f, 1.5f, 1.1f, Color{ 14, 16, 20, 255 });
                if ((idxJ + i) % 2 == 0) {
                    DrawCube(Vector3{ fx + (ps[i].x < 0 ? 0.02f : -0.02f), wy + 0.52f, wz - 0.30f }, 0.05f, 0.42f, 0.45f, Color{ 115, 130, 140, 190 });
                    DrawCube(Vector3{ fx + (ps[i].x < 0 ? 0.02f : -0.02f), wy - 0.48f, wz + 0.28f }, 0.05f, 0.48f, 0.42f, Color{ 115, 130, 140, 190 });
                } else {
                    DrawCube(Vector3{ fx + (ps[i].x < 0 ? 0.01f : -0.01f), wy, wz }, 0.04f, 1.42f, 1.02f, Color{ 68, 78, 86, 210 });
                }
            }
        }
    }

            // Rua limpa sem postes
    desenharCarroAbandonado(-2.8f, -14.0f,  14.0f, Color{ 82, 46, 42, 255 });
    desenharCarroAbandonado( 2.9f,   7.5f, -165.0f, Color{ 52, 64, 72, 255 });
    desenharCarroAbandonado(-2.4f,  23.0f,  -8.0f, Color{ 76, 74, 62, 255 });
}

int main() {
    InitWindow(0, 0, "Shadow Protocol 3D");
    SetTargetFPS(30);
    shaderConsole = LoadShaderFromMemory(vsCode, fsCode);
    int locLightPos = GetShaderLocation(shaderConsole, "lightPos");
    int locLightDir = GetShaderLocation(shaderConsole, "lightDir");

    enum EstadoJogo { TELA_LAUNCHER, TELA_OPCOES, TELA_JOGANDO };
    EstadoJogo estadoAtual = TELA_LAUNCHER;
    bool optLanterna = true, optSom = true, optMostrarFps = true;
    int optFps = 30, optNivelRes = 0;
    float escalasRes[] = { 1.0f, 0.75f, 0.50f };

    carregarConfig(optLanterna, optFps, optSom, optMostrarFps, optNivelRes);
    SetTargetFPS(optFps);
    SetMasterVolume(optSom ? 1.0f : 0.0f);

    int resW = (int)(GetRenderWidth() * escalasRes[optNivelRes]);
    int resH = (int)(GetRenderHeight() * escalasRes[optNivelRes]);
    if (resW < 320) resW = 320;
    if (resH < 180) resH = 180;
    RenderTexture2D targetRes = LoadRenderTexture(resW, resH);
    SetTextureFilter(targetRes.texture, TEXTURE_FILTER_BILINEAR);

    Camera3D camera = { 0 };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 52.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    Vector3 posHeroi = { 0.0f, 0.0f, 0.0f };
    float anguloHeroi = 180.0f;
    bool agachado = false;

    while (!WindowShouldClose()) {
        float sw = (float)GetRenderWidth();
        float sh = (float)GetRenderHeight();

        if (estadoAtual != TELA_JOGANDO) {
            BeginDrawing();
            ClearBackground(Color{ 12, 16, 20, 255 });
            static bool tocouAntes = false;
            bool tocandoAgora = (GetTouchPointCount() > 0) || IsMouseButtonDown(MOUSE_LEFT_BUTTON);
            Vector2 toque = (GetTouchPointCount() > 0) ? GetTouchPosition(0) : GetMousePosition();
            bool clicou = (tocandoAgora && !tocouAntes) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
            tocouAntes = tocandoAgora;

            DrawRectangleLinesEx(Rectangle{ 24, 24, sw - 48, sh - 48 }, 3, Color{ 50, 200, 120, 200 });
            const char* titulo = "SHADOW PROTOCOL 3D";
            DrawText(titulo, (int)(sw/2 - MeasureText(titulo, 44)/2), 55, 44, Color{ 90, 245, 160, 255 });

            if (estadoAtual == TELA_LAUNCHER) {
                Rectangle rJogar = { sw/2 - 210, sh/2 - 75, 420, 80 };
                Rectangle rOpcoes = { sw/2 - 210, sh/2 + 25, 420, 80 };
                DrawRectangleRounded(rJogar, 0.25f, 10, Color{ 32, 145, 85, 255 });
                DrawRectangleRoundedLinesEx(rJogar, 0.25f, 10, 3, WHITE);
                DrawText("JOGAR", (int)(rJogar.x + 210 - MeasureText("JOGAR", 34)/2), (int)(rJogar.y + 23), 34, WHITE);
                DrawRectangleRounded(rOpcoes, 0.25f, 10, Color{ 36, 48, 60, 255 });
                DrawRectangleRoundedLinesEx(rOpcoes, 0.25f, 10, 3, Color{ 130, 210, 175, 255 });
                DrawText("OPCOES", (int)(rOpcoes.x + 210 - MeasureText("OPCOES", 34)/2), (int)(rOpcoes.y + 23), 34, WHITE);
                if (clicou && CheckCollisionPointRec(toque, rJogar)) estadoAtual = TELA_JOGANDO;
                if (clicou && CheckCollisionPointRec(toque, rOpcoes)) estadoAtual = TELA_OPCOES;
            }
            else {
                float bw = 580.0f, bh = 58.0f, bx = sw/2 - bw/2;
                Rectangle botoes[] = { {bx,125,bw,bh}, {bx,193,bw,bh}, {bx,261,bw,bh}, {bx,329,bw,bh}, {bx,397,bw,bh} };
                Rectangle rVoltar = { sw/2 - 190, sh - 102, 380, 64 };
                char txtRes[64];
                const char* nomeRes[] = { "100% (NATIVA)", "75% (ALTA)", "50% (720P PS2)" };
                std::sprintf(txtRes, "RESOLUCAO 3D: %s (%dx%d)", nomeRes[optNivelRes], resW, resH);
                const char* textos[] = {
                    optLanterna ? "GRAFICOS: ALTO (LANTERNA + NEBLINA)" : "GRAFICOS: BAIXO (MODO RAPIDO)",
                    (optFps == 60) ? "LIMITE DE FPS: 60 FPS" : "LIMITE DE FPS: 30 FPS",
                    optSom ? "SOM: LIGADO" : "SOM: MUDO",
                    optMostrarFps ? "CONTADOR FPS: VISIVEL" : "CONTADOR FPS: OCULTO",
                    txtRes
                };
                for (int i = 0; i < 5; i++) {
                    DrawRectangleRounded(botoes[i], 0.22f, 8, Color{ 30, 42, 52, 255 });
                    DrawRectangleRoundedLinesEx(botoes[i], 0.22f, 8, 2, Color{ 95, 165, 140, 220 });
                    int fs = (i == 4) ? 23 : 25;
                    DrawText(textos[i], (int)(botoes[i].x + bw/2 - MeasureText(textos[i], fs)/2), (int)(botoes[i].y + (bh - fs)/2), fs, WHITE);
                }
                DrawRectangleRounded(rVoltar, 0.25f, 10, Color{ 32, 145, 85, 255 });
                DrawText("SALVAR E VOLTAR", (int)(rVoltar.x + 190 - MeasureText("SALVAR E VOLTAR", 26)/2), (int)(rVoltar.y + 19), 26, WHITE);
                if (clicou) {
                    bool mudou = false;
                    if (CheckCollisionPointRec(toque, botoes[0])) { optLanterna = !optLanterna; mudou = true; }
                    if (CheckCollisionPointRec(toque, botoes[1])) { optFps = (optFps == 30) ? 60 : 30; SetTargetFPS(optFps); mudou = true; }
                    if (CheckCollisionPointRec(toque, botoes[2])) { optSom = !optSom; SetMasterVolume(optSom ? 1.0f : 0.0f); mudou = true; }
                    if (CheckCollisionPointRec(toque, botoes[3])) { optMostrarFps = !optMostrarFps; mudou = true; }
                    if (CheckCollisionPointRec(toque, botoes[4])) {
                        optNivelRes = (optNivelRes + 1) % 3;
                        resW = (int)(GetRenderWidth() * escalasRes[optNivelRes]);
                        resH = (int)(GetRenderHeight() * escalasRes[optNivelRes]);
                        if (resW < 320) resW = 320; if (resH < 180) resH = 180;
                        UnloadRenderTexture(targetRes); targetRes = LoadRenderTexture(resW, resH);
                        SetTextureFilter(targetRes.texture, TEXTURE_FILTER_BILINEAR); mudou = true;
                    }
                    if (mudou) salvarConfig(optLanterna, optFps, optSom, optMostrarFps, optNivelRes);
                    if (CheckCollisionPointRec(toque, rVoltar)) { salvarConfig(optLanterna, optFps, optSom, optMostrarFps, optNivelRes); estadoAtual = TELA_LAUNCHER; }
                }
            }
            EndDrawing();
            continue;
        }

        Rectangle btnMenuHud = { sw - 290.0f, 24.0f, 150.0f, 46.0f };
        Vector2 baseAnalogico = { 230.0f, sh - 205.0f };
        float raioBase = 128.0f, eixoX = 0.0f, eixoY = 0.0f;
        Vector2 pinoAnalogico = baseAnalogico;
        Vector2 posBtnA = { sw - 275.0f, sh - 125.0f }, posBtnB = { sw - 180.0f, sh - 215.0f };
        Vector2 posBtnX = { sw - 370.0f, sh - 215.0f }, posBtnY = { sw - 275.0f, sh - 305.0f };
        float raioBtn = 52.0f;
        bool btnAPress = false, btnBPress = false, btnXPress = false, btnYPress = false;

        int tc = GetTouchPointCount();
        for (int t = 0; t < tc; t++) {
            Vector2 tp = GetTouchPosition(t);
            if (CheckCollisionPointRec(tp, btnMenuHud)) estadoAtual = TELA_LAUNCHER;
            float d = std::hypot(tp.x - baseAnalogico.x, tp.y - baseAnalogico.y);
            if (d < raioBase * 1.75f && tp.x < sw * 0.45f) {
                float dx = tp.x - baseAnalogico.x, dy = tp.y - baseAnalogico.y;
                if (d > raioBase) { dx = (dx/d)*raioBase; dy = (dy/d)*raioBase; }
                pinoAnalogico = { baseAnalogico.x + dx, baseAnalogico.y + dy };
                eixoX = dx / raioBase; eixoY = dy / raioBase;
            }
            if (CheckCollisionPointCircle(tp, posBtnA, raioBtn * 1.18f)) btnAPress = true;
            if (CheckCollisionPointCircle(tp, posBtnB, raioBtn * 1.18f)) btnBPress = true;
            if (CheckCollisionPointCircle(tp, posBtnX, raioBtn * 1.18f)) btnXPress = true;
            if (CheckCollisionPointCircle(tp, posBtnY, raioBtn * 1.18f)) btnYPress = true;
        }
        if (tc == 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp, btnMenuHud)) estadoAtual = TELA_LAUNCHER;
            float d = std::hypot(mp.x - baseAnalogico.x, mp.y - baseAnalogico.y);
            if (d < raioBase * 1.75f && mp.x < sw * 0.45f) {
                float dx = mp.x - baseAnalogico.x, dy = mp.y - baseAnalogico.y;
                if (d > raioBase) { dx = (dx/d)*raioBase; dy = (dy/d)*raioBase; }
                pinoAnalogico = { baseAnalogico.x + dx, baseAnalogico.y + dy };
                eixoX = dx / raioBase; eixoY = dy / raioBase;
            }
            if (CheckCollisionPointCircle(mp, posBtnA, raioBtn * 1.18f)) btnAPress = true;
            if (CheckCollisionPointCircle(mp, posBtnB, raioBtn * 1.18f)) btnBPress = true;
            if (CheckCollisionPointCircle(mp, posBtnX, raioBtn * 1.18f)) btnXPress = true;
            if (CheckCollisionPointCircle(mp, posBtnY, raioBtn * 1.18f)) btnYPress = true;
        }

        static bool bAnt = false;
        if (btnBPress && !bAnt) agachado = !agachado;
        bAnt = btnBPress;

        float dt = GetFrameTime();
        if (std::abs(eixoX) > 0.15f) anguloHeroi -= eixoX * 140.0f * dt;
        static float passoAnim = 0.0f;
        if (std::abs(eixoY) > 0.15f) {
            float rad = anguloHeroi * DEG2RAD;
            float vel = (-eixoY) * (agachado ? 2.2f : 4.5f) * dt;
            float nx = posHeroi.x + std::sin(rad) * vel;
            float nz = posHeroi.z + std::cos(rad) * vel;
            bool bateuCarro = (std::hypot(nx - (-2.8f), nz - (-14.0f)) < 1.8f) ||
                              (std::hypot(nx - ( 2.9f), nz - (  7.5f)) < 1.8f) ||
                              (std::hypot(nx - (-2.4f), nz - ( 23.0f)) < 1.8f);
            if (!bateuCarro && nx > -13.2f && nx < 13.2f) posHeroi.x = nx;
            if (!bateuCarro && nz > -38.5f && nz < 38.5f) posHeroi.z = nz;
            passoAnim = (float)GetTime() * 10.0f;
        }

        float radCam = anguloHeroi * DEG2RAD;
        float dirX = std::sin(radCam), dirZ = std::cos(radCam);
        float rightX = -dirZ, rightZ = dirX;
        float altOlhos = agachado ? 1.15f : 1.52f;
        Vector3 alvoCam = { posHeroi.x - (dirX * 2.45f) + (rightX * 0.58f), altOlhos + 0.12f, posHeroi.z - (dirZ * 2.45f) + (rightZ * 0.58f) };
        if (alvoCam.x < -12.8f) alvoCam.x = -12.8f; if (alvoCam.x > 12.8f) alvoCam.x = 12.8f;
        if (alvoCam.z < -38.5f) alvoCam.z = -38.5f; if (alvoCam.z > 38.5f) alvoCam.z = 38.5f;
        camera.position = alvoCam;
        camera.target = { posHeroi.x + (dirX * 3.5f) + (rightX * 0.30f), altOlhos - 0.12f, posHeroi.z + (dirZ * 3.5f) + (rightZ * 0.30f) };

        float posLuz[3] = { posHeroi.x - (rightX * 0.15f) + (dirX * 0.22f), altOlhos - 0.32f, posHeroi.z - (rightZ * 0.15f) + (dirZ * 0.22f) };
        float dirLuz[3] = { dirX, -0.16f, dirZ };
        SetShaderValue(shaderConsole, locLightPos, posLuz, SHADER_UNIFORM_VEC3);
        SetShaderValue(shaderConsole, locLightDir, dirLuz, SHADER_UNIFORM_VEC3);
                int intLanterna = optLanterna ? 1 : 0;
                SetShaderValue(shaderConsole, GetShaderLocation(shaderConsole, "lanternaOn"), &intLanterna, SHADER_UNIFORM_INT);

        BeginTextureMode(targetRes);
        ClearBackground(Color{ 8, 10, 16, 255 });
        BeginMode3D(camera);
            if (optLanterna) BeginShaderMode(shaderConsole);
            contadorCarroChamada = 0;
                desenharRuaSilentHill();
            if (optLanterna) EndShaderMode();
            desenharBonecoPS1(posHeroi, anguloHeroi, passoAnim, agachado);
        EndMode3D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(targetRes.texture, Rectangle{ 0.0f, 0.0f, (float)targetRes.texture.width, -(float)targetRes.texture.height }, Rectangle{ 0.0f, 0.0f, sw, sh }, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

        DrawCircleV(baseAnalogico, raioBase, Color{ 20, 26, 32, 195 });
        DrawCircleLines((int)baseAnalogico.x, (int)baseAnalogico.y, raioBase, Color{ 115, 140, 160, 235 });
        DrawCircleV(pinoAnalogico, 54.0f, Color{ 58, 68, 78, 240 });
        DrawCircleV(pinoAnalogico, 26.0f, Color{ 36, 42, 50, 245 });

        auto btnXbox = [&](Vector2 c, const char* txt, Color cor, bool on) {
            DrawCircleV(c, raioBtn, Color{ 14, 18, 22, 225 });
            DrawCircleV(c, raioBtn - 5.0f, on ? WHITE : Color{ cor.r, cor.g, cor.b, 225 });
            int lw = MeasureText(txt, 34);
            DrawText(txt, (int)c.x - lw/2, (int)c.y - 17, 34, on ? BLACK : WHITE);
        };
        btnXbox(posBtnA, "A", Color{ 46, 180, 75, 255 }, btnAPress);
        btnXbox(posBtnB, "B", Color{ 210, 52, 52, 255 }, btnBPress);
        btnXbox(posBtnX, "X", Color{ 48, 125, 220, 255 }, btnXPress);
        btnXbox(posBtnY, "Y", Color{ 230, 180, 35, 255 }, btnYPress);

        DrawRectangleRounded(btnMenuHud, 0.25f, 8, Color{ 35, 45, 55, 225 });
        DrawText("MENU", (int)(btnMenuHud.x + 75 - MeasureText("MENU", 22)/2), (int)(btnMenuHud.y + 12), 22, WHITE);
        if (optMostrarFps) DrawFPS(30, 25);
        EndDrawing();
    }
    UnloadShader(shaderConsole);
    CloseWindow();
    return 0;
}
