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

void desenharBonecoPS1(Vector3 pos, float angulo, float cicloPasso, bool agachado) {
    rlPushMatrix();
        rlTranslatef(pos.x, pos.y, pos.z);
        rlRotatef(angulo, 0.0f, 1.0f, 0.0f);

        Color corJaqueta   = { 48, 62, 74, 255 };
        Color corCamiseta  = { 210, 212, 208, 255 };
        Color corJeans     = { 32, 44, 68, 255 };
        Color corPele      = { 215, 178, 148, 255 };
        Color corCabelo    = { 58, 40, 28, 255 };
        Color corSapato    = { 38, 28, 22, 255 };
        Color corContorno  = { 12, 14, 18, 255 };

        bool andando = (cicloPasso > 0.01f);
        float tempo = (float)GetTime();

        float balancoPerna  = andando ? std::sin(cicloPasso) * (agachado ? 18.0f : 32.0f) : 0.0f;
        float balancoBraco  = andando ? std::sin(cicloPasso) * 26.0f : std::sin(tempo * 2.0f) * 2.5f;
        float puloPasso     = andando ? std::abs(std::cos(cicloPasso)) * 0.045f : 0.0f;
        float respiracao    = !andando ? std::sin(tempo * 2.6f) * 0.015f : 0.0f;
        float gingadoTronco = andando ? std::sin(cicloPasso) * 4.5f : 0.0f;

        float alturaQuadril    = agachado ? 0.52f : (0.78f + puloPasso);
        float inclinacaoTronco = agachado ? 22.0f : (andando ? 6.0f : 0.0f);

        // Perna Esquerda
        rlPushMatrix();
            rlTranslatef(-0.13f, alturaQuadril, 0.0f);
            rlRotatef(balancoPerna - (agachado ? 35.0f : 0.0f), 1.0f, 0.0f, 0.0f);
            DrawCube({ 0.0f, -0.34f, 0.0f }, 0.19f, 0.66f, 0.21f, corJeans);
            DrawCubeWires({ 0.0f, -0.34f, 0.0f }, 0.19f, 0.66f, 0.21f, corContorno);
            DrawCube({ 0.0f, -0.70f, 0.04f }, 0.20f, 0.14f, 0.29f, corSapato);
            DrawCubeWires({ 0.0f, -0.70f, 0.04f }, 0.20f, 0.14f, 0.29f, corContorno);
        rlPopMatrix();

        // Perna Direita
        rlPushMatrix();
            rlTranslatef(0.13f, alturaQuadril, 0.0f);
            rlRotatef(-balancoPerna - (agachado ? 35.0f : 0.0f), 1.0f, 0.0f, 0.0f);
            DrawCube({ 0.0f, -0.34f, 0.0f }, 0.19f, 0.66f, 0.21f, corJeans);
            DrawCubeWires({ 0.0f, -0.34f, 0.0f }, 0.19f, 0.66f, 0.21f, corContorno);
            DrawCube({ 0.0f, -0.70f, 0.04f }, 0.20f, 0.14f, 0.29f, corSapato);
            DrawCubeWires({ 0.0f, -0.70f, 0.04f }, 0.20f, 0.14f, 0.29f, corContorno);
        rlPopMatrix();

        // Hierarquia do Tronco
        rlPushMatrix();
            rlTranslatef(0.0f, alturaQuadril + respiracao, 0.0f);
            rlRotatef(inclinacaoTronco, 1.0f, 0.0f, 0.0f);
            rlRotatef(gingadoTronco, 0.0f, 0.0f, 1.0f);

            float centroTorsoY = 0.34f;
            DrawCube({ 0.0f, centroTorsoY, 0.0f }, 0.48f, 0.66f, 0.26f, corJaqueta);
            DrawCubeWires({ 0.0f, centroTorsoY, 0.0f }, 0.48f, 0.66f, 0.26f, corContorno);
            DrawCube({ 0.0f, centroTorsoY + 0.02f, 0.135f }, 0.16f, 0.60f, 0.02f, corCamiseta);
            DrawCube({ -0.14f, centroTorsoY + 0.30f, 0.12f }, 0.12f, 0.10f, 0.06f, corJaqueta);
            DrawCube({  0.14f, centroTorsoY + 0.30f, 0.12f }, 0.12f, 0.10f, 0.06f, corJaqueta);

            // Lanterna no bolso
            DrawCube({ -0.15f, centroTorsoY + 0.12f, 0.15f }, 0.07f, 0.16f, 0.07f, Color{ 30, 32, 35, 255 });
            DrawCube({ -0.15f, centroTorsoY + 0.15f, 0.19f }, 0.06f, 0.06f, 0.02f, Color{ 255, 245, 200, 255 });

            // Cabeca
            rlPushMatrix();
                rlTranslatef(0.0f, 0.84f, 0.0f);
                rlRotatef(-inclinacaoTronco * 0.6f, 1.0f, 0.0f, 0.0f);
                DrawCube({ 0.0f, 0.0f, 0.0f }, 0.28f, 0.31f, 0.28f, corPele);
                DrawCubeWires({ 0.0f, 0.0f, 0.0f }, 0.28f, 0.31f, 0.28f, corContorno);
                DrawCube({ -0.07f, 0.02f, 0.142f }, 0.04f, 0.03f, 0.01f, Color{ 25, 20, 20, 255 });
                DrawCube({  0.07f, 0.02f, 0.142f }, 0.04f, 0.03f, 0.01f, Color{ 25, 20, 20, 255 });
                DrawCube({ 0.0f, 0.13f, -0.01f }, 0.30f, 0.11f, 0.31f, corCabelo);
                DrawCube({ 0.0f, 0.02f, -0.14f }, 0.30f, 0.26f, 0.06f, corCabelo);
                DrawCube({ -0.14f, 0.04f, -0.02f }, 0.04f, 0.18f, 0.24f, corCabelo);
                DrawCube({  0.14f, 0.04f, -0.02f }, 0.04f, 0.18f, 0.24f, corCabelo);
            rlPopMatrix();

            // Braco Esquerdo
            rlPushMatrix();
                rlTranslatef(-0.31f, 0.58f, 0.0f);
                rlRotatef(-balancoBraco, 1.0f, 0.0f, 0.0f);
                rlRotatef(4.0f, 0.0f, 0.0f, 1.0f);
                DrawCube({ 0.0f, -0.25f, 0.0f }, 0.15f, 0.50f, 0.16f, corJaqueta);
                DrawCubeWires({ 0.0f, -0.25f, 0.0f }, 0.15f, 0.50f, 0.16f, corContorno);
                DrawCube({ 0.0f, -0.54f, 0.01f }, 0.11f, 0.12f, 0.13f, corPele);
            rlPopMatrix();

            // Braco Direito
            rlPushMatrix();
                rlTranslatef(0.31f, 0.58f, 0.0f);
                rlRotatef(balancoBraco, 1.0f, 0.0f, 0.0f);
                rlRotatef(-4.0f, 0.0f, 0.0f, 1.0f);
                DrawCube({ 0.0f, -0.25f, 0.0f }, 0.15f, 0.50f, 0.16f, corJaqueta);
                DrawCubeWires({ 0.0f, -0.25f, 0.0f }, 0.15f, 0.50f, 0.16f, corContorno);
                DrawCube({ 0.0f, -0.54f, 0.01f }, 0.11f, 0.12f, 0.13f, corPele);
            rlPopMatrix();
        rlPopMatrix();
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
    "varying vec3 fragPos;\n"
    "varying vec2 fragTexCoord;\n"
    "varying vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform vec3 lightPos;\n"
    "uniform vec3 lightDir;\n"
    "void main() {\n"
    "    vec4 texel = texture2D(texture0, fragTexCoord) * colDiffuse * fragColor;\n"
    "    vec3 toFrag = fragPos - lightPos;\n"
    "    float dist = length(toFrag);\n"
    "    float spot = dot(normalize(toFrag), normalize(lightDir));\n"
    "    float cone = smoothstep(0.74, 0.96, spot); float nucleo = smoothstep(0.90, 0.995, spot);\n"
    "    float atten = clamp(1.0 - dist / 22.0, 0.0, 1.0);\n"
    "    float feixe = (cone * 1.15 + nucleo * 0.95) * atten;
        vec3 corLuz = vec3(0.10, 0.11, 0.13) + vec3(1.0, 0.93, 0.76) * feixe;\n"
    "    // Becos laterais bem escuros quando longe da avenida central\n"
    "    if (abs(fragPos.x) > 10.6) corLuz *= 0.45;\n"
    "    // Neblina estilo Silent Hill (densa e cinza-azulada)\n"
    "    float neblina = smoothstep(3.5, 25.0, dist) * (1.0 - clamp(feixe * 0.55, 0.0, 0.65));\n"
    "    vec3 finalRgb = mix(texel.rgb * corLuz, vec3(0.38, 0.42, 0.46), neblina);\n"
    "    gl_FragColor = vec4(finalRgb, texel.a);\n"
    "}\n";

Model modAsfalto, modCalcada, modPredio;
bool modelosRuaProntos = false;


void desenharCarroAbandonado(float x, float z, float rotY, Color corLataria) {
    rlPushMatrix();
        rlTranslatef(x, 0.0f, z);
        rlRotatef(rotY, 0.0f, 1.0f, 0.0f);

        Color corVidro     = { 18, 22, 28, 255 };
        Color corPneu      = { 16, 16, 18, 255 };
        Color corRoda      = { 95, 100, 108, 255 };
        Color corParachoque= { 35, 38, 42, 255 };
        Color corFarol     = { 185, 180, 155, 255 };
        Color corLanternaT = { 120, 22, 22, 255 };
        Color corContorno  = { 10, 12, 15, 255 };

        // Base / Chassi inferior
        DrawCube({ 0.0f, 0.52f, 0.0f }, 1.92f, 0.56f, 4.20f, corLataria);
        DrawCubeWires({ 0.0f, 0.52f, 0.0f }, 1.92f, 0.56f, 4.20f, corContorno);

        // Cabine / Teto
        DrawCube({ 0.0f, 1.05f, -0.15f }, 1.66f, 0.52f, 2.25f, corLataria);
        DrawCubeWires({ 0.0f, 1.05f, -0.15f }, 1.66f, 0.52f, 2.25f, corContorno);

        // Vidros (Para-brisa, traseiro e laterais)
        DrawCube({ 0.0f, 1.04f,  0.98f }, 1.50f, 0.40f, 0.04f, corVidro);
        DrawCube({ 0.0f, 1.04f, -1.28f }, 1.50f, 0.40f, 0.04f, corVidro);
        DrawCube({ -0.84f, 1.04f, -0.15f }, 0.04f, 0.38f, 1.95f, corVidro);
        DrawCube({  0.84f, 1.04f, -0.15f }, 0.04f, 0.38f, 1.95f, corVidro);

        // Para-choques dianteiro e traseiro
        DrawCube({ 0.0f, 0.36f,  2.14f }, 1.98f, 0.22f, 0.16f, corParachoque);
        DrawCube({ 0.0f, 0.36f, -2.14f }, 1.98f, 0.22f, 0.16f, corParachoque);

        // Farois e Lanternas traseiras
        DrawCube({ -0.66f, 0.58f,  2.11f }, 0.36f, 0.16f, 0.04f, corFarol);
        DrawCube({  0.66f, 0.58f,  2.11f }, 0.36f, 0.16f, 0.04f, corFarol);
        DrawCube({ -0.66f, 0.58f, -2.11f }, 0.36f, 0.16f, 0.04f, corLanternaT);
        DrawCube({  0.66f, 0.58f, -2.11f }, 0.36f, 0.16f, 0.04f, corLanternaT);

        // 4 Rodas
        float posRodasX[2] = { -0.94f, 0.94f };
        float posRodasZ[2] = {  1.30f, -1.30f };
        for (int ix = 0; ix < 2; ix++) {
            for (int iz = 0; iz < 2; iz++) {
                DrawCube({ posRodasX[ix], 0.32f, posRodasZ[iz] }, 0.22f, 0.62f, 0.62f, corPneu);
                DrawCube({ posRodasX[ix] * 1.02f, 0.32f, posRodasZ[iz] }, 0.23f, 0.32f, 0.32f, corRoda);
            }
        }
    rlPopMatrix();
}

void desenharRuaSilentHill() {
    if (!modelosRuaProntos) {
        Image i1 = GenImageChecked(64, 64, 8, 8, Color{34,36,40,255}, Color{42,44,48,255});
        modAsfalto = LoadModelFromMesh(GenMeshPlane(12.0f, 80.0f, 1, 1));
        modAsfalto.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(i1);
        UnloadImage(i1);
        Image i2 = GenImageChecked(32, 32, 8, 8, Color{88,92,96,255}, Color{100,104,110,255});
        modCalcada = LoadModelFromMesh(GenMeshCube(4.4f, 0.24f, 80.0f));
        modCalcada.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(i2);
        UnloadImage(i2);
        Image i3 = GenImageChecked(32, 32, 4, 8, Color{72,76,80,255}, Color{60,64,68,255});
        modPredio = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
        modPredio.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(i3);
        UnloadImage(i3);
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

    // Postes com lampadas amareladas fracas
    for (float pz = -30.0f; pz <= 30.0f; pz += 15.0f) {
        for (int l = -1; l <= 1; l += 2) {
            float px = l * 6.4f;
            DrawCylinder(Vector3{ px, 0.24f, pz }, 0.12f, 0.16f, 5.5f, 8, Color{ 44, 48, 52, 255 });
            DrawCube(Vector3{ px - l*0.7f, 5.65f, pz }, 1.5f, 0.10f, 0.12f, Color{ 44, 48, 52, 255 });
            // Lampada amarela fraca + cone de luz suave no chao
            DrawCube(Vector3{ px - l*1.3f, 5.55f, pz }, 0.38f, 0.14f, 0.24f, Color{ 255, 225, 140, 255 });
            DrawSphere(Vector3{ px - l*1.3f, 5.48f, pz }, 0.26f, Color{ 255, 225, 145, 55 });
        }
    }

    // Latas de lixo nas calcadas e entrada dos becos
    float lixoPos[][2] = { {-6.8f,-19.0f}, {-7.2f,-18.3f}, {6.9f,-16.5f}, {-7.0f,0.5f}, {7.1f,1.2f}, {-6.8f,18.5f}, {7.0f,19.2f} };
    for (int i = 0; i < 7; i++) {
        DrawCylinder(Vector3{ lixoPos[i][0], 0.24f, lixoPos[i][1] }, 0.28f, 0.25f, 0.85f, 10, Color{ 72, 78, 82, 255 });
        DrawCylinderWires(Vector3{ lixoPos[i][0], 0.24f, lixoPos[i][1] }, 0.28f, 0.25f, 0.85f, 10, Color{ 22, 24, 26, 255 });
        if (i % 2 == 0) DrawCylinder(Vector3{ lixoPos[i][0], 1.09f, lixoPos[i][1] }, 0.30f, 0.29f, 0.08f, 10, Color{ 85, 90, 95, 255 });
    }

    // Carros abandonados na pista
    desenharCarroAbandonado(-2.8f, -14.0f,  14.0f, Color{ 82, 46, 42, 255 });
    desenharCarroAbandonado( 2.9f,   7.5f, -165.0f, Color{ 52, 64, 72, 255 });
    desenharCarroAbandonado(-2.4f,  23.0f,  -8.0f, Color{ 76, 74, 62, 255 });
}

int main() {
    InitWindow(0, 0, "Shadow Protocol 3D");
    SetTargetFPS(30);
    Shader shaderConsole = LoadShaderFromMemory(vsCode, fsCode);
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

        BeginTextureMode(targetRes);
        ClearBackground(Color{ 97, 107, 117, 255 });
        BeginMode3D(camera);
            if (optLanterna) BeginShaderMode(shaderConsole);
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
