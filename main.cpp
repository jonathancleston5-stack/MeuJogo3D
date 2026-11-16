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
        Color corJaqueta = { 48, 62, 74, 255 }, corCamiseta = { 210, 212, 208, 255 };
        Color corJeans = { 32, 44, 68, 255 }, corPele = { 215, 178, 148, 255 };
        Color corCabelo = { 58, 40, 28, 255 }, corSapato = { 38, 28, 22, 255 };
        Color corContorno = { 12, 14, 18, 255 };
        rlScalef(1.0f, agachado ? 0.72f : 1.0f, 1.0f);
        float balanco = (cicloPasso > 0.0f) ? std::sin(cicloPasso) * 28.0f : 0.0f;
        float resp = (cicloPasso == 0.0f) ? std::sin((float)GetTime() * 2.8f) * 0.015f : 0.0f;
        for (int lado = -1; lado <= 1; lado += 2) {
            rlPushMatrix();
                rlTranslatef(0.13f * lado, 0.78f, 0.0f);
                rlRotatef(balanco * (-lado), 1.0f, 0.0f, 0.0f);
                DrawCube({ 0.0f, -0.35f, 0.0f }, 0.19f, 0.68f, 0.21f, corJeans);
                DrawCubeWires({ 0.0f, -0.35f, 0.0f }, 0.19f, 0.68f, 0.21f, corContorno);
                DrawCube({ 0.0f, -0.72f, 0.04f }, 0.20f, 0.14f, 0.28f, corSapato);
            rlPopMatrix();
        }
        float yTorso = 1.12f + resp;
        DrawCube({ 0.0f, yTorso, 0.0f }, 0.48f, 0.66f, 0.26f, corJaqueta);
        DrawCubeWires({ 0.0f, yTorso, 0.0f }, 0.48f, 0.66f, 0.26f, corContorno);
        DrawCube({ 0.0f, yTorso + 0.02f, 0.135f }, 0.16f, 0.60f, 0.02f, corCamiseta);
        DrawCube({ -0.15f, yTorso + 0.12f, 0.15f }, 0.07f, 0.16f, 0.07f, Color{ 30, 32, 35, 255 });
        DrawCube({ -0.15f, yTorso + 0.15f, 0.19f }, 0.06f, 0.06f, 0.02f, Color{ 255, 245, 200, 255 });
        float yCabeca = 1.63f + resp;
        DrawCube({ 0.0f, yCabeca, 0.0f }, 0.28f, 0.31f, 0.28f, corPele);
        DrawCubeWires({ 0.0f, yCabeca, 0.0f }, 0.28f, 0.31f, 0.28f, corContorno);
        DrawCube({ 0.0f, yCabeca + 0.13f, -0.01f }, 0.30f, 0.11f, 0.31f, corCabelo);
        DrawCube({ 0.0f, yCabeca + 0.02f, -0.14f }, 0.30f, 0.26f, 0.06f, corCabelo);
        for (int lado = -1; lado <= 1; lado += 2) {
            rlPushMatrix();
                rlTranslatef(0.31f * lado, yTorso + 0.24f, 0.0f);
                rlRotatef(balanco * 0.85f * lado, 1.0f, 0.0f, 0.0f);
                DrawCube({ 0.0f, -0.26f, 0.0f }, 0.15f, 0.52f, 0.16f, corJaqueta);
                DrawCubeWires({ 0.0f, -0.26f, 0.0f }, 0.15f, 0.52f, 0.16f, corContorno);
                DrawCube({ 0.0f, -0.55f, 0.0f }, 0.12f, 0.12f, 0.13f, corPele);
            rlPopMatrix();
        }
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
    "    float cone = smoothstep(0.68, 0.94, spot);\n"
    "    float atten = clamp(1.0 - dist / 24.0, 0.0, 1.0);\n"
    "    float luz = 0.28 + (cone * atten * 0.95);\n"
    "    float neblina = clamp((dist - 5.0) / 26.0, 0.0, 0.92);\n"
    "    vec3 finalRgb = mix(texel.rgb * luz, vec3(0.56, 0.60, 0.64), neblina);\n"
    "    gl_FragColor = vec4(finalRgb, texel.a);\n"
    "}\n";

Model modAsfalto, modCalcada, modPredio;
bool modelosRuaProntos = false;

void desenharRuaSilentHill() {
    if (!modelosRuaProntos) {
        Image i1 = GenImageChecked(64, 64, 8, 8, Color{36,38,42,255}, Color{44,46,50,255});
        modAsfalto = LoadModelFromMesh(GenMeshPlane(12.0f, 80.0f, 1, 1));
        modAsfalto.materials.maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(i1);
        UnloadImage(i1);
        Image i2 = GenImageChecked(32, 32, 8, 8, Color{92,95,100,255}, Color{104,108,114,255});
        modCalcada = LoadModelFromMesh(GenMeshCube(4.4f, 0.24f, 80.0f));
        modCalcada.materials.maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(i2);
        UnloadImage(i2);
        Image i3 = GenImageChecked(32, 32, 4, 8, Color{78,82,86,255}, Color{66,70,74,255});
        modPredio = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
        modPredio.materials.maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(i3);
        UnloadImage(i3);
        modelosRuaProntos = true;
    }
    DrawModel(modAsfalto, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, Color{ 165, 170, 175, 255 });
    for (float z = -36.0f; z <= 36.0f; z += 4.5f)
        DrawCube(Vector3{ 0.0f, 0.015f, z }, 0.22f, 0.02f, 2.2f, Color{ 195, 165, 55, 255 });
    DrawModel(modCalcada, Vector3{ -8.2f, 0.12f, 0.0f }, 1.0f, Color{ 195, 200, 205, 255 });
    DrawModel(modCalcada, Vector3{  8.2f, 0.12f, 0.0f }, 1.0f, Color{ 195, 200, 205, 255 });
    struct Predio { float x, z, w, h, d; Color cor; };
    Predio ps[] = {
        {-14.5f,-28.0f,8.5f,16.0f,16.0f,{85,88,92,255}}, {-14.5f,-10.0f,8.5f,21.0f,17.0f,{95,90,85,255}},
        {-14.5f,  9.0f,8.5f,14.0f,18.0f,{80,85,90,255}}, {-14.5f, 28.0f,8.5f,19.0f,16.0f,{90,88,84,255}},
        { 14.5f,-27.0f,8.5f,18.0f,17.0f,{88,85,82,255}}, { 14.5f, -8.0f,8.5f,15.0f,16.0f,{78,84,90,255}},
        { 14.5f, 10.0f,8.5f,22.0f,17.0f,{92,88,84,255}}, { 14.5f, 28.0f,8.5f,16.0f,16.0f,{82,86,90,255}}
    };
    for (int i = 0; i < 8; i++) {
        DrawModelEx(modPredio, Vector3{ ps[i].x, ps[i].h/2.0f, ps[i].z }, Vector3{0,1,0}, 0.0f, Vector3{ ps[i].w, ps[i].h, ps[i].d }, ps[i].cor);
        float fx = (ps[i].x < 0) ? (ps[i].x + ps[i].w/2.0f + 0.04f) : (ps[i].x - ps[i].w/2.0f - 0.04f);
        for (float wy = 3.5f; wy < ps[i].h - 2.0f; wy += 3.2f)
            for (float wz = ps[i].z - ps[i].d/2.0f + 2.5f; wz <= ps[i].z + ps[i].d/2.0f - 2.5f; wz += 3.5f)
                DrawCube(Vector3{ fx, wy, wz }, 0.08f, 1.5f, 1.1f, Color{ 22, 26, 30, 255 });
    }
    for (float pz = -30.0f; pz <= 30.0f; pz += 15.0f) {
        for (int l = -1; l <= 1; l += 2) {
            float px = l * 6.4f;
            DrawCylinder(Vector3{ px, 0.24f, pz }, 0.12f, 0.16f, 5.5f, 8, Color{ 48, 52, 56, 255 });
            DrawCube(Vector3{ px - l*0.7f, 5.65f, pz }, 1.5f, 0.10f, 0.12f, Color{ 48, 52, 56, 255 });
        }
    }
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

        Rectangle btnMenuHud = { sw - 170.0f, 20.0f, 150.0f, 44.0f };
        Vector2 baseAnalogico = { 185.0f, sh - 185.0f };
        float raioBase = 120.0f, eixoX = 0.0f, eixoY = 0.0f;
        Vector2 pinoAnalogico = baseAnalogico;
        Vector2 posBtnA = { sw - 195.0f, sh - 110.0f }, posBtnB = { sw - 110.0f, sh - 195.0f };
        Vector2 posBtnX = { sw - 280.0f, sh - 195.0f }, posBtnY = { sw - 195.0f, sh - 280.0f };
        float raioBtn = 45.0f;
        bool btnAPress = false, btnBPress = false, btnXPress = false, btnYPress = false;

        int tc = GetTouchPointCount();
        for (int t = 0; t < tc; t++) {
            Vector2 tp = GetTouchPosition(t);
            if (CheckCollisionPointRec(tp, btnMenuHud)) estadoAtual = TELA_LAUNCHER;
            float d = std::hypot(tp.x - baseAnalogico.x, tp.y - baseAnalogico.y);
            if (d < raioBase * 1.7f && tp.x < sw * 0.45f) {
                float dx = tp.x - baseAnalogico.x, dy = tp.y - baseAnalogico.y;
                if (d > raioBase) { dx = (dx/d)*raioBase; dy = (dy/d)*raioBase; }
                pinoAnalogico = { baseAnalogico.x + dx, baseAnalogico.y + dy };
                eixoX = dx / raioBase; eixoY = dy / raioBase;
            }
            if (CheckCollisionPointCircle(tp, posBtnA, raioBtn * 1.2f)) btnAPress = true;
            if (CheckCollisionPointCircle(tp, posBtnB, raioBtn * 1.2f)) btnBPress = true;
            if (CheckCollisionPointCircle(tp, posBtnX, raioBtn * 1.2f)) btnXPress = true;
            if (CheckCollisionPointCircle(tp, posBtnY, raioBtn * 1.2f)) btnYPress = true;
        }
        if (tc == 0 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mp = GetMousePosition();
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mp, btnMenuHud)) estadoAtual = TELA_LAUNCHER;
            float d = std::hypot(mp.x - baseAnalogico.x, mp.y - baseAnalogico.y);
            if (d < raioBase * 1.7f && mp.x < sw * 0.45f) {
                float dx = mp.x - baseAnalogico.x, dy = mp.y - baseAnalogico.y;
                if (d > raioBase) { dx = (dx/d)*raioBase; dy = (dy/d)*raioBase; }
                pinoAnalogico = { baseAnalogico.x + dx, baseAnalogico.y + dy };
                eixoX = dx / raioBase; eixoY = dy / raioBase;
            }
            if (CheckCollisionPointCircle(mp, posBtnA, raioBtn * 1.2f)) btnAPress = true;
            if (CheckCollisionPointCircle(mp, posBtnB, raioBtn * 1.2f)) btnBPress = true;
            if (CheckCollisionPointCircle(mp, posBtnX, raioBtn * 1.2f)) btnXPress = true;
            if (CheckCollisionPointCircle(mp, posBtnY, raioBtn * 1.2f)) btnYPress = true;
        }

        static bool bAnt = false;
        if (btnBPress && !bAnt) agachado = !agachado;
        bAnt = btnBPress;

        float dt = GetFrameTime();
        if (std::abs(eixoX) > 0.15f) anguloHeroi -= eixoX * 140.0f * dt;
        float passoAnim = 0.0f;
        if (std::abs(eixoY) > 0.15f) {
            float rad = anguloHeroi * DEG2RAD;
            float vel = (-eixoY) * (agachado ? 2.2f : 4.5f) * dt;
            float nx = posHeroi.x + std::sin(rad) * vel;
            float nz = posHeroi.z + std::cos(rad) * vel;
            if (nx > -9.4f && nx < 9.4f) posHeroi.x = nx;
            if (nz > -38.5f && nz < 38.5f) posHeroi.z = nz;
            passoAnim = (float)GetTime() * 10.0f;
        }

        float radCam = anguloHeroi * DEG2RAD;
        float dirX = std::sin(radCam), dirZ = std::cos(radCam);
        float rightX = -dirZ, rightZ = dirX;
        float altOlhos = agachado ? 1.15f : 1.52f;
        Vector3 alvoCam = { posHeroi.x - (dirX * 2.45f) + (rightX * 0.58f), altOlhos + 0.12f, posHeroi.z - (dirZ * 2.45f) + (rightZ * 0.58f) };
        if (alvoCam.x < -9.2f) alvoCam.x = -9.2f; if (alvoCam.x > 9.2f) alvoCam.x = 9.2f;
        if (alvoCam.z < -38.5f) alvoCam.z = -38.5f; if (alvoCam.z > 38.5f) alvoCam.z = 38.5f;
        camera.position = alvoCam;
        camera.target = { posHeroi.x + (dirX * 3.5f) + (rightX * 0.30f), altOlhos - 0.12f, posHeroi.z + (dirZ * 3.5f) + (rightZ * 0.30f) };

        float posLuz[] = { posHeroi.x, altOlhos, posHeroi.z };
        float dirLuz[] = { dirX, -0.12f, dirZ };
        SetShaderValue(shaderConsole, locLightPos, posLuz, SHADER_UNIFORM_VEC3);
        SetShaderValue(shaderConsole, locLightDir, dirLuz, SHADER_UNIFORM_VEC3);

        BeginTextureMode(targetRes);
        ClearBackground(Color{ 142, 152, 162, 255 });
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

        DrawCircleV(baseAnalogico, raioBase, Color{ 20, 26, 32, 185 });
        DrawCircleLines((int)baseAnalogico.x, (int)baseAnalogico.y, raioBase, Color{ 100, 125, 145, 230 });
        DrawCircleV(pinoAnalogico, 50.0f, Color{ 58, 68, 78, 240 });
        DrawCircleV(pinoAnalogico, 24.0f, Color{ 36, 42, 50, 245 });

        auto btnXbox = [&](Vector2 c, const char* txt, Color cor, bool on) {
            DrawCircleV(c, raioBtn, Color{ 14, 18, 22, 220 });
            DrawCircleV(c, raioBtn - 4.0f, on ? WHITE : Color{ cor.r, cor.g, cor.b, 220 });
            int lw = MeasureText(txt, 32);
            DrawText(txt, (int)c.x - lw/2, (int)c.y - 16, 32, on ? BLACK : WHITE);
        };
        btnXbox(posBtnA, "A", Color{ 46, 180, 75, 255 }, btnAPress);
        btnXbox(posBtnB, "B", Color{ 210, 52, 52, 255 }, btnBPress);
        btnXbox(posBtnX, "X", Color{ 48, 125, 220, 255 }, btnXPress);
        btnXbox(posBtnY, "Y", Color{ 230, 180, 35, 255 }, btnYPress);

        DrawRectangleRounded(btnMenuHud, 0.25f, 8, Color{ 35, 45, 55, 220 });
        DrawText("MENU", (int)(btnMenuHud.x + 75 - MeasureText("MENU", 20)/2), (int)(btnMenuHud.y + 12), 20, WHITE);
        if (optMostrarFps) DrawFPS(30, 25);
        EndDrawing();
    }
    UnloadShader(shaderConsole);
    CloseWindow();
    return 0;
}
