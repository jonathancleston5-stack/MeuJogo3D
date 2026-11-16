#include "raylib.h"
#include "rlgl.h"
#include <cmath>
#include <vector>

struct Bala {
    Vector3 pos;
    Vector3 vel;
    bool doJogador;
    bool ativa;
};

struct Guarda {
    Vector3 pos;
    float angulo;
    float cicloPasso;
    int vida;
    int estado; // 0 = Patrulha, 1 = Investigando som/pista, 2 = Alerta/Combate
    int pontoRota;
    int tempoPausa;
    int cooldownTiro;
    Vector3 alvoInvestigar;
};

Texture2D gerarTexturaCimento() {
    Image img = GenImageColor(256, 256, { 88, 92, 96, 255 });
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            int ruido = ((x * 73 + y * 151) % 21) - 10;
            unsigned char c = (unsigned char)(90 + ruido);
            Color cor = { c, (unsigned char)(c + 2), (unsigned char)(c + 5), 255 };
            if (x < 3 || y < 3 || (x > 126 && x < 129) || (y > 126 && y < 129)) cor = { 52, 55, 58, 255 };
            ImageDrawPixel(&img, x, y, cor);
        }
    }
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D gerarTexturaCaixa() {
    Image img = GenImageColor(128, 128, { 138, 98, 56, 255 });
    for (int y = 0; y < 128; y += 16) ImageDrawRectangle(&img, 0, y, 128, 2, { 82, 54, 28, 255 });
    ImageDrawRectangle(&img, 0, 0, 128, 10, { 102, 70, 38, 255 });
    ImageDrawRectangle(&img, 0, 118, 128, 10, { 102, 70, 38, 255 });
    ImageDrawRectangle(&img, 0, 0, 10, 128, { 102, 70, 38, 255 });
    ImageDrawRectangle(&img, 118, 0, 10, 128, { 102, 70, 38, 255 });
    for (int i = 0; i < 128; i++) ImageDrawRectangle(&img, i, i - 4, 8, 8, { 110, 76, 42, 255 });
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Sound criarSomAlerta() {
    const int sr = 44100;
    const int frames = sr * 0.28;
    short* dados = (short*)MemAlloc(frames * sizeof(short));
    for (int i = 0; i < frames; i++) {
        double t = (double)i / sr;
        double freq = (i < frames / 3) ? 1480.0 : 1960.0;
        dados[i] = (short)(std::sin(2.0 * PI * freq * t) * 11000);
    }
    Wave w = { (unsigned int)frames, (unsigned int)sr, 16, 1, dados };
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

Sound criarSomSilenciador() {
    const int sr = 44100;
    const int frames = sr * 0.12;
    short* dados = (short*)MemAlloc(frames * sizeof(short));
    for (int i = 0; i < frames; i++) {
        double env = 1.0 - ((double)i / frames);
        dados[i] = (short)((GetRandomValue(-10000, 10000)) * env * env);
    }
    Wave w = { (unsigned int)frames, (unsigned int)sr, 16, 1, dados };
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

Sound criarSomAK47() {
    const int sr = 44100;
    const int frames = sr * 0.14;
    short* dados = (short*)MemAlloc(frames * sizeof(short));
    for (int i = 0; i < frames; i++) {
        double t = (double)i / sr;
        double env = 1.0 - ((double)i / frames);
        dados[i] = (short)((std::sin(2.0 * PI * 140.0 * t) * 14000 + GetRandomValue(-12000, 12000)) * env);
    }
    Wave w = { (unsigned int)frames, (unsigned int)sr, 16, 1, dados };
    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}
Model modPistola = { 0 };
Model modRifle   = { 0 };

void desenharBonecoPS1(Vector3 pos, float anguloGraus, float cicloPasso, bool agachado, bool vivo, bool ehJogador) {
    // Sombra de contato no chao (tira o aspecto de boneco flutuando)
    DrawCylinder(Vector3{ pos.x, 0.01f, pos.z }, 0.48f, 0.48f, 0.01f, 16, Fade(BLACK, 0.45f));

    rlPushMatrix();
        rlTranslatef(pos.x, 0.0f, pos.z);
        rlRotatef(anguloGraus, 0.0f, 1.0f, 0.0f);

        if (!vivo) {
            rlTranslatef(0.0f, 0.22f, 0.0f);
            rlRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        }

        Color corRoupa   = ehJogador ? Color{ 42, 62, 85, 255 }   : Color{ 135, 38, 38, 255 };
        Color corColete  = ehJogador ? Color{ 25, 32, 42, 255 }   : Color{ 55, 25, 25, 255 };
        Color corPele    = Color{ 218, 178, 142, 255 };
        Color corDetalhe = ehJogador ? Color{ 30, 95, 175, 255 }  : Color{ 215, 50, 50, 255 };
        Color corBota    = Color{ 22, 24, 28, 255 };

        float balanco = vivo ? (std::sin(cicloPasso) * 26.0f) : 0.0f;
        float alturaQuadril = agachado ? 0.56f : 0.88f;
        float altPerna = agachado ? 0.52f : 0.84f;

        // Perna Esquerda (coxa + joelho + bota arredondada)
        rlPushMatrix();
            rlTranslatef(-0.20f, alturaQuadril, 0.0f);
            rlRotatef(agachado ? -35.0f : balanco, 1.0f, 0.0f, 0.0f);
            DrawCapsule(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, -altPerna + 0.16f, 0.0f }, 0.13f, 8, 8, corRoupa);
            DrawCylinder(Vector3{ 0.0f, -altPerna, 0.04f }, 0.13f, 0.15f, 0.22f, 10, corBota);
        rlPopMatrix();

        // Perna Direita
        rlPushMatrix();
            rlTranslatef(0.20f, alturaQuadril, 0.0f);
            rlRotatef(agachado ? -35.0f : -balanco, 1.0f, 0.0f, 0.0f);
            DrawCapsule(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, -altPerna + 0.16f, 0.0f }, 0.13f, 8, 8, corRoupa);
            DrawCylinder(Vector3{ 0.0f, -altPerna, 0.04f }, 0.13f, 0.15f, 0.22f, 10, corBota);
        rlPopMatrix();

        // Quadril e Cinto Tatico
        DrawCylinder(Vector3{ 0.0f, alturaQuadril - 0.05f, 0.0f }, 0.30f, 0.28f, 0.18f, 12, corBota);

        // Tronco arredondado (mais largo nos ombros, fino na cintura) + Colete Tatico
        float yTronco = alturaQuadril + 0.36f;
        DrawCylinder(Vector3{ 0.0f, yTronco - 0.32f, 0.0f }, 0.36f, 0.28f, 0.68f, 12, corRoupa);
        DrawCylinder(Vector3{ 0.0f, yTronco - 0.24f, 0.02f }, 0.38f, 0.31f, 0.50f, 12, corColete);
        // Mochila/Equipamento nas costas
        DrawCube(Vector3{ 0.0f, yTronco + 0.04f, -0.28f }, 0.36f, 0.42f, 0.16f, corBota);

        // Braco Esquerdo (ombro esferico + braco cilindrico + luva)
        rlPushMatrix();
            rlTranslatef(-0.44f, yTronco + 0.26f, 0.0f);
            DrawSphere(Vector3{ 0.0f, 0.0f, 0.0f }, 0.14f, corRoupa);
            rlRotatef(-balanco * 0.8f, 1.0f, 0.0f, 0.0f);
            DrawCapsule(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, -0.52f, 0.0f }, 0.10f, 8, 8, corRoupa);
            DrawSphere(Vector3{ 0.0f, -0.58f, 0.0f }, 0.10f, corBota);
        rlPopMatrix();

        // Braco Direito + Arma detalhada (Pistola com silenciador ou Fuzil)
        rlPushMatrix();
            rlTranslatef(0.44f, yTronco + 0.26f, 0.0f);
            DrawSphere(Vector3{ 0.0f, 0.0f, 0.0f }, 0.14f, corRoupa);
            rlRotatef(vivo ? -90.0f : 0.0f, 1.0f, 0.0f, 0.0f);
            DrawCapsule(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, -0.50f, 0.0f }, 0.10f, 8, 8, corRoupa);
            DrawSphere(Vector3{ 0.0f, -0.56f, 0.0f }, 0.10f, corBota);
                                           DrawModelEx(modPistola, Vector3{ 0.0f, -0.62f, 0.05f }, Vector3{ 0.0f, 1.0f, 0.0f }, 90.0f, Vector3{ 0.012f, 0.012f, 0.012f }, WHITE);
 


        rlPopMatrix();

        float yCabeca = yTronco + 0.62f;
        DrawSphere(Vector3{ 0.0f, yCabeca, 0.0f }, 0.24f, corPele);
        if (ehJogador) {
            
            // Cabelo + Bandana tatica com pontas atras
            DrawCylinder(Vector3{ 0.0f, yCabeca + 0.06f, 0.0f }, 0.24f, 0.25f, 0.16f, 12, Color{ 55, 42, 32, 255 });
            DrawCylinder(Vector3{ 0.0f, yCabeca + 0.04f, 0.0f }, 0.255f, 0.255f, 0.08f, 12, corDetalhe);
            DrawCube(Vector3{ -0.06f, yCabeca - 0.04f, -0.26f }, 0.06f, 0.22f, 0.04f, corDetalhe);
            DrawCube(Vector3{  0.06f, yCabeca - 0.02f, -0.26f }, 0.06f, 0.18f, 0.04f, corDetalhe);
        } else {
            // Capacete militar + Viseira luminosa
            DrawSphere(Vector3{ 0.0f, yCabeca + 0.05f, -0.02f }, 0.255f, Color{ 45, 48, 52, 255 });
            DrawCube(Vector3{ 0.0f, yCabeca + 0.03f, 0.22f }, 0.28f, 0.09f, 0.08f, corDetalhe);
        }
    rlPopMatrix();
}


bool pontoBloqueado3D(float x, float z, const std::vector<BoundingBox>& caixas) {
    for (const auto& box : caixas) {
        if (x >= box.min.x && x <= box.max.x && z >= box.min.z && z <= box.max.z) return true;
    }
    return false;
    
}
// Shader embutido de Iluminacao Direcional + Nevoa (Fog) estilo Console
const char* vsCodigo = R"(
#version 100
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;
varying vec3 fragPos;
void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragPos = vec3(matModel * vec4(vertexPosition, 1.0));
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

const char* fsCodigo = R"(
#version 100
precision mediump float;
varying vec2 fragTexCoord;
varying vec4 fragColor;
varying vec3 fragNormal;
varying vec3 fragPos;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;
uniform vec3 spotPos;
uniform vec3 spotDir;
void main() {
    vec4 texel = texture2D(texture0, fragTexCoord) * colDiffuse * fragColor;
    vec3 norm = normalize(fragNormal);

    // Vetor da lanterna ate o ponto da parede/chao
    vec3 toFrag = fragPos - spotPos;
    float distSpot = length(toFrag);
    vec3 dirToFrag = normalize(toFrag);

    // Cone de luz da lanterna (Silent Hill)
    float theta = dot(dirToFrag, normalize(spotDir));
    float cone = smoothstep(0.76, 0.94, theta);
    float atenuacao = clamp(1.0 - (distSpot / 15.0), 0.0, 1.0);
    atenuacao = atenuacao * atenuacao;

    float diff = max(dot(norm, -dirToFrag), 0.15);
    vec3 luzLanterna = vec3(1.15, 1.08, 0.92) * cone * atenuacao * diff * 2.2;

    // Escuridao ambiente bem baixa fora do feixe
    vec3 ambient = vec3(0.04, 0.05, 0.06);
    vec3 colorLit = texel.rgb * (ambient + luzLanterna);

    // Nevoa escura no fundo
    float distCam = length(viewPos - fragPos);
    float fogFactor = clamp((distCam - 10.0) / 24.0, 0.0, 0.85);
    vec3 finalColor = mix(colorLit, vec3(0.02, 0.03, 0.03), fogFactor);
    gl_FragColor = vec4(finalColor, texel.a);
}
)";


void Bloco(Vector3 origem, float x, float y, float z, Vector3 tamanho, Color cor) {
    DrawCubeV({ origem.x + x, origem.y + y, origem.z + z }, tamanho, cor);
}

void DesenharCenarioIndustrial(Vector3 origem = { 0, 0, 0 }) {
    Color metal       = { 38, 46, 51, 255 };
    Color metalClaro  = { 85, 99, 105, 255 };
    Color concreto    = { 65, 70, 73, 255 };
    Color amarelo     = { 190, 155, 55, 255 };

    // 1. LINHAS DE SEGURANÇA E CANALETAS NO CHÃO
    for (int i = -9; i <= 9; i += 2) {
        Bloco(origem, (float)i, 0.015f, -9.5f, { 0.14f, 0.02f, 1.8f }, amarelo);
    }
    for (int x = -8; x <= 8; x += 8) {
        Bloco(origem, (float)x, 0.012f, 0.0f, { 0.16f, 0.02f, 22.0f }, metal);
    }

    // 2. PAREDES EXTERNAS (alinhadas com a colisão -11.25 a +11.25)
    Bloco(origem, 0.0f, 2.5f, -11.35f, { 23.0f, 5.0f, 0.4f }, metal);
    Bloco(origem, -11.35f, 2.5f, 0.0f, { 0.4f, 5.0f, 23.0f }, concreto);
    Bloco(origem,  11.35f, 2.5f, 0.0f, { 0.4f, 5.0f, 23.0f }, concreto);

    // 3. PAINÉIS METÁLICOS NAS PAREDES
    for (int i = -8; i <= 8; i += 4) {
        if (i != 8) {
            Bloco(origem, (float)i, 2.3f, -11.12f, { 3.2f, 3.6f, 0.05f }, metalClaro);
        }
        Bloco(origem, -11.12f, 2.3f, (float)i, { 0.05f, 3.6f, 3.2f }, metalClaro);
        Bloco(origem,  11.12f, 2.3f, (float)i, { 0.05f, 3.6f, 3.2f }, metalClaro);
    }

    // 4. PILARES NOS CANTOS E NAS PAREDES
    float cantosX[2] = { -10.9f, 10.9f };
    float cantosZ[3] = { -10.9f, 0.0f, 10.9f };
    for (int ix = 0; ix < 2; ix++) {
        for (int iz = 0; iz < 3; iz++) {
            Bloco(origem, cantosX[ix], 2.5f, cantosZ[iz], { 0.75f, 5.0f, 0.75f }, metal);
            Bloco(origem, cantosX[ix], 0.3f, cantosZ[iz], { 1.05f, 0.6f, 1.05f }, metalClaro);
        }
    }

    // 5. TUBULAÇÕES E DETALHES INDUSTRIAIS NAS PAREDES
    DrawCylinderEx(
        { origem.x - 10.95f, origem.y + 4.1f, origem.z - 11.0f },
        { origem.x - 10.95f, origem.y + 4.1f, origem.z + 11.0f },
        0.11f, 0.11f, 8, metalClaro
    );
    DrawCylinderEx(
        { origem.x - 11.0f, origem.y + 4.1f, origem.z - 10.95f },
        { origem.x + 11.0f, origem.y + 4.1f, origem.z - 10.95f },
        0.11f, 0.11f, 8, metalClaro
    );

    // Armário metálico lateral e placa de advertência
    Bloco(origem, -10.6f, 1.3f, 3.5f, { 0.9f, 2.6f, 1.3f }, metal);
    Bloco(origem, -10.12f, 1.3f, 3.5f, { 0.06f, 2.3f, 0.95f }, metalClaro);
    Bloco(origem, 0.0f, 2.2f, -11.08f, { 2.4f, 0.75f, 0.06f }, amarelo);
}



int main() {
    const int LARGURA = 960;
    const int ALTURA = 432;
    SetConfigFlags(FLAG_FULLSCREEN_MODE); 
    InitWindow(0, 0, "Novo Metal Gear - IA Tatica");
    InitAudioDevice();
    SetTargetFPS(30);

    Texture2D texCimento = gerarTexturaCimento();
    Texture2D texCaixa   = gerarTexturaCaixa();
        GenTextureMipmaps(&texCimento);
    SetTextureFilter(texCimento, TEXTURE_FILTER_TRILINEAR);
    GenTextureMipmaps(&texCaixa);
    SetTextureFilter(texCaixa, TEXTURE_FILTER_TRILINEAR);
        modPistola = LoadModel("armas/pistola.obj");
    Texture2D texPistola = LoadTexture("armas/Pistol_01_Albedo.png");
    modPistola.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texPistola;

    modRifle = LoadModel("armas/rifle.obj");
    Texture2D texRifle = LoadTexture("armas/Rifle_01_Albedo.png");
    modRifle.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texRifle;

    Sound somAlerta      = criarSomAlerta();
    Sound somPistola     = criarSomSilenciador();
    Sound somAK47        = criarSomAK47();

    Model modChao        = LoadModelFromMesh(GenMeshPlane(24.0f, 24.0f, 1, 1));
    modChao.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texCimento;

    Model modParedeFundo = LoadModelFromMesh(GenMeshCube(24.0f, 9.2f, 1.0f));
    modParedeFundo.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texCimento;

    Model modParedeLat   = LoadModelFromMesh(GenMeshCube(1.0f, 9.2f, 24.0f));
    modParedeLat.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texCimento;

    Model modCaixaGrande = LoadModelFromMesh(GenMeshCube(4.0f, 2.5f, 3.0f));
    modCaixaGrande.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texCaixa;

    Model modCaixaBaixa  = LoadModelFromMesh(GenMeshCube(5.0f, 1.5f, 1.5f));
    modCaixaBaixa.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texCaixa;
        // Ativa iluminacao direcional e nevoa nos materiais
    Shader shaderConsole = LoadShaderFromMemory(vsCodigo, fsCodigo);
    int viewPosLoc = GetShaderLocation(shaderConsole, "viewPos");
        int spotPosLoc = GetShaderLocation(shaderConsole, "spotPos");
    int spotDirLoc = GetShaderLocation(shaderConsole, "spotDir");

    modChao.materials->shader = shaderConsole;
    modCaixaGrande.materials->shader = shaderConsole;
    modCaixaBaixa.materials->shader = shaderConsole;
    modPistola.materials[0].shader = shaderConsole;
        // Aplica as texturas industriais geradas no chao e nas paredes
    Texture2D texChaoNova   = LoadTexture("chao_grade.png");
    Texture2D texParedeNova = LoadTexture("parede_metal.png");
    SetTextureFilter(texChaoNova, TEXTURE_FILTER_POINT);
    SetTextureFilter(texParedeNova, TEXTURE_FILTER_POINT);

    modChao.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texChaoNova;
    modParedeFundo.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texParedeNova;
    modParedeLat.materials->maps[MATERIAL_MAP_DIFFUSE].texture = texParedeNova;

    modParedeFundo.materials->shader = shaderConsole;
    modParedeLat.materials->shader = shaderConsole;

    Vector3 posSnake = { 0.0f, 0.0f, 9.0f };
    float anguloSnake = 180.0f;
    float cicloPassoSnake = 0.0f;
    float velocidadeBase = 0.15f;
    bool agachado = false;
    int vidaSnake = 100;

    Camera3D camera = { 0 };
    camera.position = { 0.0f, 13.5f, 18.0f };
    camera.target = posSnake;
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    std::vector<BoundingBox> obstaculos = {
        { { -7.0f, 0.0f, -2.0f }, { -3.0f, 2.5f, 1.0f } },
        { {  3.0f, 0.0f, -2.0f }, {  7.0f, 2.5f, 1.0f } },
        { { -2.5f, 0.0f,  3.0f }, {  2.5f, 1.5f, 4.5f } }
    };

    BoundingBox boxElevador = { { 7.0f, 0.0f, -11.5f }, { 10.0f, 3.2f, -9.8f } };

    // Rota em circuito (4 cantos ao redor da área central)
    std::vector<Vector3> rotaCircuito = {
        { -8.5f, 0.0f, -6.5f },
        {  8.5f, 0.0f, -6.5f },
        {  8.5f, 0.0f,  2.0f },
        { -8.5f, 0.0f,  2.0f }
    };

    // 2 Guardas em pontos opostos do circuito
    std::vector<Guarda> guardas = {
        { { -8.5f, 0.0f, -6.5f }, 90.0f, 0.0f, 100, 0, 1, 0, 0, { 0, 0, 0 } },
        { {  8.5f, 0.0f,  2.0f }, -90.0f, 0.0f, 100, 0, 3, 0, 0, { 0, 0, 0 } }
    };

    std::vector<Bala> balas;
    int tempoAlertaGlobal = 0;
        bool modoPrimeiraPessoa = false;
    Rectangle btnCamera = { (float)LARGURA - 145.0f, 15.0f, 130.0f, 38.0f };

        float tamBtn = 95.0f;
    Rectangle btnCima     = { 145.0f, (float)ALTURA - 290.0f, tamBtn, tamBtn };
    Rectangle btnBaixo    = { 145.0f, (float)ALTURA - 115.0f, tamBtn, tamBtn };
    Rectangle btnEsquerda = {  50.0f, (float)ALTURA - 202.0f, tamBtn, tamBtn };
    Rectangle btnDireita  = { 240.0f, (float)ALTURA - 202.0f, tamBtn, tamBtn };

    Vector2 centroBtnAgachar = { (float)LARGURA - 245.0f, (float)ALTURA - 115.0f };
    float raioBtnAgachar     = 55.0f;

    Vector2 centroBtnAtirar  = { (float)LARGURA - 115.0f, (float)ALTURA - 195.0f };
    float raioBtnAtirar      = 68.0f;

    
    enum EstadoJogo { TELA_LAUNCHER, TELA_OPCOES, TELA_JOGANDO };
    EstadoJogo estadoAtual = TELA_LAUNCHER;
    bool optLanterna = true;
    int optFps = 30;
    bool optSom = true;
    bool optMostrarFps = true;
    int optNivelRes = 0; // 0 = 100% Nativa, 1 = 75% (1080p), 2 = 50% (720p)
    float escalasRes[3] = { 1.0f, 0.75f, 0.50f };
    int resW = GetScreenWidth();
    int resH = GetScreenHeight();
    RenderTexture2D targetRes = LoadRenderTexture(resW, resH);
    SetTextureFilter(targetRes.texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose()) {
        float sw = (float)GetScreenWidth();
        float sh = (float)GetScreenHeight();
        centroBtnAtirar = { sw - 125.0f, sh - 125.0f };
        centroBtnAgachar = { sw - 265.0f, sh - 95.0f };
        btnCamera = { sw - 170.0f, 20.0f, 150.0f, 44.0f };
        Rectangle btnMenuHud = { sw - 335.0f, 20.0f, 150.0f, 44.0f };

        if (estadoAtual != TELA_JOGANDO) {
            BeginDrawing();
            ClearBackground(Color{ 14, 18, 22, 255 });
            Vector2 toque = GetMousePosition();
            bool clicou = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

            DrawRectangleLinesEx(Rectangle{ 20, 20, sw - 40, sh - 40 }, 2, Color{ 45, 180, 110, 180 });
            const char* titulo = "SHADOW PROTOCOL 3D";
            DrawText(titulo, (int)(sw/2 - MeasureText(titulo, 36)/2), 65, 36, Color{ 80, 235, 150, 255 });

            if (estadoAtual == TELA_LAUNCHER) {
                Rectangle rJogar = { sw/2 - 160, sh/2 - 55, 320, 62 };
                Rectangle rOpcoes = { sw/2 - 160, sh/2 + 25, 320, 62 };

                DrawRectangleRounded(rJogar, 0.25f, 8, Color{ 35, 130, 80, 255 });
                DrawRectangleRoundedLinesEx(rJogar, 0.25f, 8, 2, WHITE);
                DrawText("JOGAR", (int)(rJogar.x + 160 - MeasureText("JOGAR", 28)/2), (int)(rJogar.y + 17), 28, WHITE);

                DrawRectangleRounded(rOpcoes, 0.25f, 8, Color{ 35, 48, 58, 255 });
                DrawRectangleRoundedLinesEx(rOpcoes, 0.25f, 8, 2, Color{ 120, 190, 160, 255 });
                DrawText("OPCOES", (int)(rOpcoes.x + 160 - MeasureText("OPCOES", 28)/2), (int)(rOpcoes.y + 17), 28, WHITE);

                if (clicou && CheckCollisionPointRec(toque, rJogar)) estadoAtual = TELA_JOGANDO;
                if (clicou && CheckCollisionPointRec(toque, rOpcoes)) estadoAtual = TELA_OPCOES;
            } else if (estadoAtual == TELA_OPCOES) {
                Rectangle rGfx = { sw/2 - 220, 145, 440, 52 };
                Rectangle rFps = { sw/2 - 220, 210, 440, 52 };
                Rectangle rSom = { sw/2 - 220, 275, 440, 52 };
                Rectangle rShowFps = { sw/2 - 220, 340, 440, 52 };
                Rectangle rRes = { sw/2 - 220, 405, 440, 52 };
                Rectangle rVoltar = { sw/2 - 140, sh - 105, 280, 55 };

                const char* txtGfx = optLanterna ? "GRAFICOS: ALTO (LANTERNA)" : "GRAFICOS: BAIXO (RAPIDO)";
                const char* txtFps = (optFps == 60) ? "LIMITE DE FPS: 60 FPS" : "LIMITE DE FPS: 30 FPS";
                const char* txtSom = optSom ? "SOM: LIGADO" : "SOM: MUDO";
                const char* txtShow = optMostrarFps ? "CONTADOR FPS: VISIVEL" : "CONTADOR FPS: OCULTO";
                char txtRes[64];
                const char* nomeRes[3] = { "100% (NATIVA)", "75% (ALTA)", "50% (720P PS2)" };
                sprintf(txtRes, "RESOLUCAO 3D: %s (%dx%d)", nomeRes[optNivelRes], resW, resH);

                DrawRectangleRounded(rGfx, 0.2f, 8, Color{ 30, 40, 48, 255 });
                DrawText(txtGfx, (int)(rGfx.x + 220 - MeasureText(txtGfx, 22)/2), (int)(rGfx.y + 15), 22, WHITE);

                DrawRectangleRounded(rFps, 0.2f, 8, Color{ 30, 40, 48, 255 });
                DrawText(txtFps, (int)(rFps.x + 220 - MeasureText(txtFps, 22)/2), (int)(rFps.y + 15), 22, WHITE);

                DrawRectangleRounded(rSom, 0.2f, 8, Color{ 30, 40, 48, 255 });
                DrawText(txtSom, (int)(rSom.x + 220 - MeasureText(txtSom, 22)/2), (int)(rSom.y + 15), 22, WHITE);

                DrawRectangleRounded(rShowFps, 0.2f, 8, Color{ 30, 40, 48, 255 });
                DrawText(txtShow, (int)(rShowFps.x + 220 - MeasureText(txtShow, 22)/2), (int)(rShowFps.y + 15), 22, WHITE);

                DrawRectangleRounded(rRes, 0.2f, 8, Color{ 30, 40, 48, 255 });
                DrawText(txtRes, (int)(rRes.x + 220 - MeasureText(txtRes, 20)/2), (int)(rRes.y + 16), 20, Color{ 100, 230, 170, 255 });

                DrawRectangleRounded(rVoltar, 0.25f, 8, Color{ 35, 130, 80, 255 });
                DrawText("VOLTAR", (int)(rVoltar.x + 140 - MeasureText("VOLTAR", 24)/2), (int)(rVoltar.y + 15), 24, WHITE);

                if (clicou) {
                    if (CheckCollisionPointRec(toque, rGfx)) optLanterna = !optLanterna;
                    if (CheckCollisionPointRec(toque, rFps)) { optFps = (optFps == 30) ? 60 : 30; SetTargetFPS(optFps); }
                    if (CheckCollisionPointRec(toque, rSom)) { optSom = !optSom; SetMasterVolume(optSom ? 1.0f : 0.0f); }
                    if (CheckCollisionPointRec(toque, rShowFps)) optMostrarFps = !optMostrarFps;
                    if (CheckCollisionPointRec(toque, rRes)) {
                        optNivelRes = (optNivelRes + 1) % 3;
                        resW = (int)(GetScreenWidth() * escalasRes[optNivelRes]);
                        resH = (int)(GetScreenHeight() * escalasRes[optNivelRes]);
                        if (resW < 320) resW = 320;
                        if (resH < 180) resH = 180;
                        UnloadRenderTexture(targetRes);
                        targetRes = LoadRenderTexture(resW, resH);
                        SetTextureFilter(targetRes.texture, TEXTURE_FILTER_BILINEAR);
                    }
                    if (CheckCollisionPointRec(toque, rVoltar)) estadoAtual = TELA_LAUNCHER;
                }
            }
                    DrawRectangleRounded(btnMenuHud, 0.25f, 8, Color{ 35, 45, 55, 220 });
        DrawRectangleRoundedLinesEx(btnMenuHud, 0.25f, 8, 2, Color{ 90, 220, 150, 255 });
        DrawText("MENU", (int)(btnMenuHud.x + 75 - MeasureText("MENU", 20)/2), (int)(btnMenuHud.y + 12), 20, WHITE);
        EndDrawing();
            continue;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), btnMenuHud)) {
            estadoAtual = TELA_LAUNCHER;
        }

        Vector3 posAnterior = posSnake;
        bool moveCima = false, moveBaixo = false, moveEsq = false, moveDir = false;

        Vector2 dedo = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(dedo, btnCima))     moveCima = true;
            if (CheckCollisionPointRec(dedo, btnBaixo))    moveBaixo = true;
            if (CheckCollisionPointRec(dedo, btnEsquerda)) moveEsq = true;
            if (CheckCollisionPointRec(dedo, btnDireita))  moveDir = true;
        }

        bool atirouAgora = false;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointCircle(dedo, centroBtnAgachar, raioBtnAgachar)) agachado = !agachado;
            if (CheckCollisionPointCircle(dedo, centroBtnAtirar, raioBtnAtirar))   atirouAgora = true;
        }

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    moveCima = true;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  moveBaixo = true;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  moveEsq = true;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) moveDir = true;
        if (IsKeyPressed(KEY_C)) agachado = !agachado;
        if (IsKeyPressed(KEY_SPACE)) atirouAgora = true;

        float velAtual = agachado ? (velocidadeBase * 0.55f) : velocidadeBase;
        float dx = 0.0f, dz = 0.0f;
        if (moveCima)  dz -= 1.0f;
        if (moveBaixo) dz += 1.0f;
        if (moveEsq)   dx -= 1.0f;
        if (moveDir)   dx += 1.0f;

                if (modoPrimeiraPessoa) {
            // Na 1a pessoa: < e > giram a visao; ^ e v andam para frente e para tras
            if (dx != 0.0f) anguloSnake -= dx * 2.8f;
            float rad = anguloSnake * DEG2RAD;
            if (dz != 0.0f) {
                float frente = -dz; // ^ (dz < 0) anda para frente
                posSnake.x += std::sin(rad) * frente * velAtual;
                posSnake.z += std::cos(rad) * frente * velAtual;
                cicloPassoSnake += 0.25f;
            } else {
                cicloPassoSnake = 0.0f;
            }
        } else {
            // Na camera alta (Tactical): movimento livre nas 4 direcoes do mapa
            if (dx != 0.0f || dz != 0.0f) {
                float len = std::sqrt(dx * dx + dz * dz);
                dx /= len;
                dz /= len;
                posSnake.x += dx * velAtual;
                posSnake.z += dz * velAtual;
                anguloSnake = std::atan2(dx, dz) * RAD2DEG;
                cicloPassoSnake += 0.25f;
            } else {
                cicloPassoSnake = 0.0f;
            }
        }

    

         if (atirouAgora) {
            PlaySound(somPistola);
            float radTiro = anguloSnake * DEG2RAD;
            float altTiro = agachado ? 0.85f : 1.35f;
            Bala b = {
                { posSnake.x + std::sin(radTiro) * 0.8f, altTiro, posSnake.z + std::cos(radTiro) * 0.8f },
                { std::sin(radTiro) * 0.45f, 0.0f, std::cos(radTiro) * 0.45f },
                true, true
            };
            balas.push_back(b);
        }

        BoundingBox boxSnake = {
            { posSnake.x - 0.55f, 0.0f, posSnake.z - 0.55f },
            { posSnake.x + 0.55f, 2.0f, posSnake.z + 0.55f }
        };

        bool bateu = (posSnake.x < -10.8f || posSnake.x > 10.8f || posSnake.z < -10.8f || posSnake.z > 10.8f);
        for (const auto& obs : obstaculos) {
            if (CheckCollisionBoxes(boxSnake, obs)) bateu = true;
        }
        if (bateu) posSnake = posAnterior;

        // Reset ao vencer no elevador ou morrer
        if ((CheckCollisionBoxes(boxSnake, boxElevador) && tempoAlertaGlobal == 0) || vidaSnake <= 0) {
            posSnake = { 0.0f, 0.0f, 9.0f };
            vidaSnake = 100;
            tempoAlertaGlobal = 0;
            balas.clear();
            guardas[0] = { { -8.5f, 0.0f, -6.5f }, 90.0f, 0.0f, 100, 0, 1, 0, 0, { 0, 0, 0 } };
            guardas[1] = { {  8.5f, 0.0f,  2.0f }, -90.0f, 0.0f, 100, 0, 3, 0, 0, { 0, 0, 0 } };
        }

                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btnCamera)) {
            modoPrimeiraPessoa = !modoPrimeiraPessoa;
        }

        float radSnake = anguloSnake * DEG2RAD;
        if (modoPrimeiraPessoa) {
            float alturaOlhos = agachado ? 0.95f : 1.65f;
            camera.position = { posSnake.x + std::sin(radSnake) * 0.25f, alturaOlhos, posSnake.z + std::cos(radSnake) * 0.25f };
            camera.target   = { posSnake.x + std::sin(radSnake) * 8.0f,  alturaOlhos, posSnake.z + std::cos(radSnake) * 8.0f };
        } else {
            camera.target   = { posSnake.x, 1.0f, posSnake.z };
            camera.position = { posSnake.x * 0.6f, 13.5f, posSnake.z + 18.0f };
        }


        // --- INTELIGÊNCIA ARTIFICIAL DOS 2 GUARDAS ---
        for (auto& g : guardas) {
            if (g.vida <= 0) continue;

            if (g.estado == 0) {
                // Estado 0: Patrulha em circuito com paradas para olhar em volta
                if (g.tempoPausa > 0) {
                    g.tempoPausa--;
                    g.cicloPasso = 0.0f;
                    g.angulo += (g.tempoPausa > 30) ? 1.5f : -1.5f; // Olha para os lados
                } else {
                    Vector3 alvo = rotaCircuito[g.pontoRota];
                    float gx = alvo.x - g.pos.x;
                    float gz = alvo.z - g.pos.z;
                    float dist = std::sqrt(gx * gx + gz * gz);

                    if (dist < 0.25f) {
                        g.pontoRota = (g.pontoRota + 1) % rotaCircuito.size();
                        g.tempoPausa = 60; // Para 1 segundo para vigiar cada esquina
                    } else {
                        g.pos.x += (gx / dist) * 0.07f;
                        g.pos.z += (gz / dist) * 0.07f;
                        g.angulo = std::atan2(gx, gz) * RAD2DEG;
                        g.cicloPasso += 0.20f;
                    }
                }
            } else if (g.estado == 1) {
                // Estado 1: Investigando ruído ou último local visto
                float gx = g.alvoInvestigar.x - g.pos.x;
                float gz = g.alvoInvestigar.z - g.pos.z;
                float dist = std::sqrt(gx * gx + gz * gz);
                if (dist < 0.5f) {
                    g.estado = 0;
                    g.tempoPausa = 70;
                } else {
                    Vector3 antG = g.pos;
                    g.pos.x += (gx / dist) * 0.09f;
                    g.pos.z += (gz / dist) * 0.09f;
                    if (pontoBloqueado3D(g.pos.x, g.pos.z, obstaculos)) { g.pos = antG; g.estado = 0; }
                    g.angulo = std::atan2(gx, gz) * RAD2DEG;
                    g.cicloPasso += 0.24f;
                }
            } else if (g.estado == 2) {
                // Estado 2: Combate com AK-47!
                g.cicloPasso = 0.0f;
                float gx = posSnake.x - g.pos.x;
                float gz = posSnake.z - g.pos.z;
                g.angulo = std::atan2(gx, gz) * RAD2DEG;

                if (g.cooldownTiro <= 0) {
                    PlaySound(somAK47);
                    float lenG = std::sqrt(gx * gx + gz * gz);
                    Bala bInimiga = {
                        { g.pos.x + (gx / lenG) * 0.9f, 1.35f, g.pos.z + (gz / lenG) * 0.9f },
                        { (gx / lenG) * 0.36f, 0.0f, (gz / lenG) * 0.36f },
                        false, true
                    };
                    balas.push_back(bInimiga);
                    g.cooldownTiro = 16;
                } else {
                    g.cooldownTiro--;
                }
            }
        }
        // Atualiza balas e verifica dano
        for (auto& b : balas) {
            if (!b.ativa) continue;
            b.pos.x += b.vel.x;
            b.pos.z += b.vel.z;

            if (b.pos.x < -11.5f || b.pos.x > 11.5f || b.pos.z < -11.5f || b.pos.z > 11.5f || pontoBloqueado3D(b.pos.x, b.pos.z, obstaculos)) {
                b.ativa = false;
                // Se a bala bater perto de um guarda vivo, ele vai investigar o barulho!
                for (auto& g : guardas) {
                    if (g.vida > 0 && g.estado == 0) {
                        float dxB = b.pos.x - g.pos.x;
                        float dzB = b.pos.z - g.pos.z;
                        if (std::sqrt(dxB * dxB + dzB * dzB) < 7.0f) {
                            g.estado = 1;
                            g.alvoInvestigar = b.pos;
                        }
                    }
                }
                continue;
            }

            if (b.doJogador) {
                for (auto& g : guardas) {
                    if (g.vida > 0 && std::fabs(b.pos.x - g.pos.x) < 0.7f && std::fabs(b.pos.z - g.pos.z) < 0.7f) {
                        b.ativa = false;
                        // Tiro furtivo (se ele não estava em combate) abate na hora!
                        g.vida -= (g.estado < 2) ? 100 : 50;
                        if (g.vida <= 0) { g.vida = 0; g.estado = 0; }
                    }
                }
            } else {
                if (std::fabs(b.pos.x - posSnake.x) < 0.65f && std::fabs(b.pos.z - posSnake.z) < 0.65f) {
                    b.ativa = false;
                    vidaSnake -= 25;
                }
            }
        }

        if (tempoAlertaGlobal > 0) {
            tempoAlertaGlobal--;
            if (tempoAlertaGlobal == 0) {
                for (auto& g : guardas) {
                    if (g.vida > 0 && g.estado == 2) {
                        g.estado = 1;
                        g.alvoInvestigar = posSnake;
                    }
                }
            }
        }
                        SetShaderValue(shaderConsole, viewPosLoc, &camera.position, SHADER_UNIFORM_VEC3);
         float radLuz = anguloSnake * DEG2RAD;
        float alturaLuz = agachado ? 0.85f : 1.35f;
        Vector3 posLanterna = { posSnake.x + std::sin(radLuz) * 0.3f, alturaLuz, posSnake.z + std::cos(radLuz) * 0.3f };
        Vector3 dirLanterna = { std::sin(radLuz), -0.12f, std::cos(radLuz) };
        SetShaderValue(shaderConsole, spotPosLoc, &posLanterna, SHADER_UNIFORM_VEC3);
        SetShaderValue(shaderConsole, spotDirLoc, &dirLanterna, SHADER_UNIFORM_VEC3);

        

        BeginDrawing();
            ClearBackground(tempoAlertaGlobal > 0 ? Color{ 65, 12, 16, 255 } : Color{ 18, 22, 26, 255 });

            
                                BeginTextureMode(targetRes);
        ClearBackground(Color{ 12, 16, 18, 255 });
        BeginMode3D(camera);
            if (optLanterna) BeginShaderMode(shaderConsole);
                DrawModel(modChao, { 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
                            DrawModel(modParedeFundo, { 0.0f, 2.5f, -11.35f }, 1.0f, WHITE);
            DrawModel(modParedeLat, { -11.35f, 2.5f, 0.0f }, 1.0f, WHITE);
            DrawModel(modParedeLat, {  11.35f, 2.5f, 0.0f }, 1.0f, WHITE);

            
                    

                DrawModel(modCaixaGrande, { -5.0f, 1.25f, -0.5f }, 1.0f, WHITE);
                DrawCubeWires({ -5.0f, 1.25f, -0.5f }, 4.0f, 2.5f, 3.0f, { 50, 32, 16, 255 });
                DrawModel(modCaixaGrande, { 5.0f, 1.25f, -0.5f }, 1.0f, WHITE);
                DrawCubeWires({ 5.0f, 1.25f, -0.5f }, 4.0f, 2.5f, 3.0f, { 50, 32, 16, 255 });
                DrawModel(modCaixaBaixa, { 0.0f, 0.75f, 3.75f }, 1.0f, WHITE);
                DrawCubeWires({ 0.0f, 0.75f, 3.75f }, 5.0f, 1.5f, 1.5f, { 50, 32, 16, 255 });
        // Porta de aco reforcada (puxada para frente da parede e iluminada pela lanterna)
        Vector3 posPorta = { 8.5f, 1.55f, -10.95f };
        DrawCube(posPorta, 2.2f, 3.1f, 0.16f, Color{ 95, 108, 118, 255 });
        DrawCubeWires(posPorta, 2.2f, 3.1f, 0.16f, Color{ 160, 175, 185, 255 });
        // Moldura metalica da porta
        DrawCube({ posPorta.x - 1.15f, 1.55f, -10.90f }, 0.14f, 3.2f, 0.22f, Color{ 120, 130, 140, 255 });
        DrawCube({ posPorta.x + 1.15f, 1.55f, -10.90f }, 0.14f, 3.2f, 0.22f, Color{ 120, 130, 140, 255 });
        DrawCube({ posPorta.x, 3.15f, -10.90f }, 2.44f, 0.16f, 0.22f, Color{ 120, 130, 140, 255 });
        // Visor e macaneta
        DrawCube({ posPorta.x, 2.15f, -10.85f }, 0.75f, 0.55f, 0.04f, Color{ 45, 75, 90, 255 });
        DrawCube({ posPorta.x + 0.75f, 1.45f, -10.83f }, 0.10f, 0.28f, 0.08f, Color{ 190, 200, 210, 255 });
        if (optLanterna) EndShaderMode();

        // Placa luminosa "SAIDA" em vermelho suave (brilha no escuro!)
        Vector3 posPlacaSaida = { posPorta.x, 3.55f, -10.88f };
        DrawCube(posPlacaSaida, 1.45f, 0.44f, 0.10f, Color{ 150, 35, 35, 255 });
        DrawCubeWires(posPlacaSaida, 1.45f, 0.44f, 0.10f, Color{ 225, 95, 95, 255 });

 
                /*
                // Cone de visão dinâmico para cada guarda vivo
                for (auto& g : guardas) {
                    if (g.vida <= 0) continue;

                    float radG = (90.0f - g.angulo) * DEG2RAD;
                    bool viuAgora = false;

                    for (float a = -0.42f; a <= 0.42f; a += 0.05f) {
                        float angRaio = radG + a;
                        float cosR = std::cos(angRaio);
                        float sinR = std::sin(angRaio);
                        Vector3 inicio = { g.pos.x, 0.05f, g.pos.z };
                        Vector3 fim = inicio;

                        for (float d = 0.5f; d <= 8.0f; d += 0.4f) {
                            float tx = g.pos.x + cosR * d;
                            float tz = g.pos.z + sinR * d;
                            if (tx < -11.2f || tx > 11.2f || tz < -11.2f || tz > 11.2f || pontoBloqueado3D(tx, tz, obstaculos)) break;
                            fim = { tx, 0.05f, tz };
                            if (std::fabs(tx - posSnake.x) < 0.7f && std::fabs(tz - posSnake.z) < 0.7f) viuAgora = true;
                        }

                        Color corFeixe = (g.estado == 2) ? Color{ 255, 50, 50, 170 } :
                                         (g.estado == 1) ? Color{ 255, 160, 40, 150 } : Color{ 245, 225, 60, 120 };
                        DrawLine3D(inicio, fim, corFeixe);
                    }

                    if (viuAgora) {
                        if (g.estado < 2) PlaySound(somAlerta);
                        g.estado = 2;
                        tempoAlertaGlobal = 150;
                    }

                    // Ícone "!" (vermelho em combate) ou "?" (laranja investigando)
                    if (g.estado == 2) {
                        DrawCube({ g.pos.x, 3.55f, g.pos.z }, 0.25f, 0.85f, 0.25f, YELLOW);
                        DrawCube({ g.pos.x, 2.85f, g.pos.z }, 0.25f, 0.25f, 0.25f, YELLOW);
                    } else if (g.estado == 1) {
                        DrawCube({ g.pos.x, 3.35f, g.pos.z }, 0.35f, 0.55f, 0.35f, ORANGE);
                    }
                }
                */
                for (const auto& b : balas) {
                    if (b.ativa) DrawSphere(b.pos, 0.14f, b.doJogador ? SKYBLUE : ORANGE);
                }
                        if (!modoPrimeiraPessoa) {
            desenharBonecoPS1(posSnake, anguloSnake, cicloPassoSnake, agachado, true, true);
                        }

               // for (const auto& g : guardas) {
                 //   desenharBonecoPS1(g.pos, g.angulo, g.cicloPasso, false, g.vida > 0, false);
               // } 
              if (modoPrimeiraPessoa) {
            float balancoArma = (cicloPassoSnake > 0.0f) ? std::sin(cicloPassoSnake * 2.0f) * 0.015f : 0.0f;

            rlPushMatrix();
                rlTranslatef(camera.position.x, camera.position.y, camera.position.z);
                rlRotatef(anguloSnake, 0.0f, 1.0f, 0.0f);
                               rlTranslatef(-0.20f, -0.22f + balancoArma, 0.42f);
                rlRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
                rlRotatef(-8.0f, 0.0f, 0.0f, 1.0f);
                DrawModel(modPistola, { 0.0f, 0.0f, 0.0f }, 0.0085f, WHITE);
 
            rlPopMatrix();
        }

            EndMode3D();
        EndTextureMode();
        DrawTexturePro(targetRes.texture,
            Rectangle{ 0.0f, 0.0f, (float)targetRes.texture.width, -(float)targetRes.texture.height },
            Rectangle{ 0.0f, 0.0f, sw, sh },
            Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);

                    // ============================================
        // HUD TÁTICA - ESTILO INFILTRAÇÃO
        // ============================================
        Color hudFundo    = { 8, 14, 18, 195 };
        Color hudVerde    = { 115, 190, 145, 235 };
        Color hudTexto    = { 220, 230, 220, 245 };
        Color hudBorda    = { 130, 165, 145, 185 };
        Color hudVermelho = { 210, 55, 45, 215 };
                // BOTÃO DE CÂMERA (1ST VIEW / TACTICAL)
        DrawRectangleRounded(btnCamera, 0.2f, 6, modoPrimeiraPessoa ? hudVerde : hudFundo);
        DrawRectangleLinesEx(btnCamera, 2, hudBorda);
        DrawText(modoPrimeiraPessoa ? "TACTICAL" : "1ST VIEW", (int)btnCamera.x + 24, (int)btnCamera.y + 12, 14, modoPrimeiraPessoa ? BLACK : hudTexto);

        // Mira central apenas na Primeira Pessoa
        if (modoPrimeiraPessoa) {
            int cx = LARGURA / 2;
            int cy = ALTURA / 2;
            DrawCircleLines(cx, cy, 8, hudBorda);
            DrawLine(cx - 14, cy, cx - 4, cy, hudVerde);
            DrawLine(cx + 4, cy, cx + 14, cy, hudVerde);
            DrawLine(cx, cy - 14, cx, cy - 4, hudVerde);
            DrawLine(cx, cy + 4, cx, cy + 14, hudVerde);
        }

        // PAINEL SUPERIOR ESQUERDO (SISTEMA + BARRA DE VIDA)
        DrawRectangleRounded(Rectangle{ 16, 15, 225, 72 }, 0.12f, 8, hudFundo);
        DrawRectangle(16, 15, 4, 72, hudVerde);
        DrawText("TACTICAL SYSTEM", 28, 22, 10, hudVerde);
        DrawText("ONLINE", 185, 22, 9, hudVerde);

        // Barra de LIFE integrada no painel
        DrawText("LIFE", 28, 40, 11, hudTexto);
        DrawRectangle(28, 55, 196, 18, Color{ 20, 30, 25, 255 });
        int larguraVida = (int)(192.0f * ((float)vidaSnake / 100.0f));
        if (larguraVida < 0) larguraVida = 0;
        Color corBarraVida = (vidaSnake > 35) ? hudVerde : hudVermelho;
        DrawRectangle(30, 57, larguraVida, 14, corBarraVida);
        DrawRectangleLines(28, 55, 196, 18, hudBorda);

        // BOTÕES DIRECIONAIS ESTILIZADOS (4 SETAS)
        DrawRectangleRounded(btnCima, 0.25f, 8, hudFundo);
        DrawRectangleLinesEx(btnCima, 2, hudBorda);
        DrawText("^", (int)(btnCima.x + btnCima.width / 2 - 7), (int)(btnCima.y + btnCima.height / 2 - 12), 26, hudTexto);

        DrawRectangleRounded(btnBaixo, 0.25f, 8, hudFundo);
        DrawRectangleLinesEx(btnBaixo, 2, hudBorda);
        DrawText("v", (int)(btnBaixo.x + btnBaixo.width / 2 - 7), (int)(btnBaixo.y + btnBaixo.height / 2 - 15), 26, hudTexto);

        DrawRectangleRounded(btnEsquerda, 0.25f, 8, hudFundo);
        DrawRectangleLinesEx(btnEsquerda, 2, hudBorda);
        DrawText("<", (int)(btnEsquerda.x + btnEsquerda.width / 2 - 7), (int)(btnEsquerda.y + btnEsquerda.height / 2 - 15), 26, hudTexto);

        DrawRectangleRounded(btnDireita, 0.25f, 8, hudFundo);
        DrawRectangleLinesEx(btnDireita, 2, hudBorda);
        DrawText(">", (int)(btnDireita.x + btnDireita.width / 2 - 7), (int)(btnDireita.y + btnDireita.height / 2 - 15), 26, hudTexto);

        // BOTÃO AGACHAR (CROUCH)
        DrawCircleV(centroBtnAgachar, raioBtnAgachar, agachado ? hudVerde : hudFundo);
        DrawCircleLines((int)centroBtnAgachar.x, (int)centroBtnAgachar.y, raioBtnAgachar, hudBorda);
        DrawCircleLines((int)centroBtnAgachar.x, (int)centroBtnAgachar.y, raioBtnAgachar - 4, hudVerde);
        DrawText("CROUCH", (int)(centroBtnAgachar.x - MeasureText("CROUCH", 12) / 2), (int)(centroBtnAgachar.y - 6), 12, agachado ? BLACK : hudTexto);

        // BOTÃO DE DISPARO (FIRE)
        DrawCircleV(centroBtnAtirar, raioBtnAtirar, hudVermelho);
        DrawCircleLines((int)centroBtnAtirar.x, (int)centroBtnAtirar.y, raioBtnAtirar, WHITE);
        DrawCircleLines((int)centroBtnAtirar.x, (int)centroBtnAtirar.y, raioBtnAtirar - 4, hudVermelho);
        DrawText("FIRE", (int)(centroBtnAtirar.x - MeasureText("FIRE", 15) / 2), (int)(centroBtnAtirar.y - 7), 15, WHITE);

        // CONTADOR DE FPS ESTILIZADO
        DrawRectangleRounded(Rectangle{ 16, 94, 92, 24 }, 0.2f, 6, hudFundo);
        DrawRectangleLinesEx(Rectangle{ 16, 94, 92, 24 }, 1, hudBorda);
        DrawText("FPS", 24, 101, 10, hudVerde);
        if (optMostrarFps) DrawFPS(56, 98);
        // Texto "SAIDA" em vermelho suave projetado sobre a placa da porta
        Vector2 telaSaida = GetWorldToScreen({ 8.5f, 3.55f, -10.98f }, camera);
                float dxP = camera.position.x - 8.5f;
        float dyP = camera.position.y - 3.55f;
        float dzP = camera.position.z - (-10.98f);
        float distPlaca = std::sqrt(dxP * dxP + dyP * dyP + dzP * dzP);

        if (distPlaca > 1.0f && distPlaca < 32.0f) {
            int tamFonte = (int)(260.0f / distPlaca);
            if (tamFonte < 9) tamFonte = 9;
            if (tamFonte > 32) tamFonte = 32;
            int largTexto = MeasureText("SAIDA", tamFonte);
            DrawText("SAIDA", (int)telaSaida.x - largTexto / 2, (int)telaSaida.y - tamFonte / 2, tamFonte, Color{ 255, 165, 165, 235 });
        }

        EndDrawing();
    }

    UnloadSound(somAlerta); UnloadSound(somPistola); UnloadSound(somAK47);
    CloseAudioDevice();
    UnloadModel(modChao); UnloadModel(modParedeFundo); UnloadModel(modParedeLat);
    UnloadModel(modCaixaGrande); UnloadModel(modCaixaBaixa);
    UnloadTexture(texCimento); UnloadTexture(texCaixa);
    CloseWindow();
    return 0;
}
