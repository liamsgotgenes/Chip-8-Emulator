#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

char opcode,delay_timer,sound_timer;
char memory[4096];
char V[16];
char fontset[]={
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
char gfx[64*32];
short I,pc,sp;
short stack[16];
int draw=0;
int wait=0;
int key_map[]={
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f,
};

void execute_cycle(){
    opcode=( (memory[pc] << 8 ) | memory[pc+1]);
    switch (opcode & 0xF000){
        case (0x000):
            switch (opcode & 0x000F){
                case (0x0000): //00E0; clear screen
                break;

                case (0x000E): //00EE; return from subroutine
                break;

                default: //0NNN;
                break;
            }
        break;

        case (0x1000): //1NNN; jump to address at NNN
        break;

        case (0x2000): //2NNN; call subroutine at NNN
        break;

        case (0x3000): //3XNN; skip the next instruction if V[X] == NN
        break;

        case (0x4000): //4XNN; skip the next instruction if V[X] != NN
        break;

        case (0x5000): //5XY0; skip the next instruction if V[X] == V[Y]
        break;

        case (0x6000): //6XNN; sets V[X] to NN
        break;

        case (0x7000): //7XNN; adds NN to V[X] without changing the carry flag
        break;

        case (0x8000):
            switch (opcode & 0x000F){
                case (0x0000): //8XY0; set V[X] to V[Y]
                break;

                case (0x0001): //8XY1; set V[X] to V[X] OR V[Y]
                break;

                case (0x0002): //8XY2; set V[X] to V[X] AND V[Y]
                break;

                case (0x0003): //8XY3; set V[X] to V[X] XOR V[Y]
                break;

                case (0x0004): //8XY4; adds V[X] to V[Y] changes the carry flag if needed
                break;

                case (0x0005): //8XY5; V[Y] is subtracted from V[X] and carry is set to 0 if there is a borrow
                break;

                case (0x0006): //8XY6; shifts V[Y] right one and stores in V[X]. Carry flag is set to LSB of V[Y] from before the shift
                break;

                case (0x0007): //8XY7; sets V[X] to V[Y]-V[X]. Carry is set to zero if there is a borrow
                break;

                case (0x000E): //8XYE; shifts V[Y] left one and stores in V[X]. Carry flag is set to LSB of V[Y] from before the shift
                break;
            }
        break;

        case (0x9000): //9XY0; skips the next instruction if V[X] != V[Y]
        break;

        case (0xA000): //A000; sets I to NNN
        break;

        case (0xB000): //BNNN; jumps to NNN + V[0]
        break;

        case (0xC000): //CXNN; sets V[X] to random number < 256 AND NN
        break;

        case (0xD000): //DXYN; draws sprite at (V[X],V[Y]) with a width of 8 and a height of N
        break;

        case (0xE000):
            switch (opcode & 0x000F){
                case (0x000E): //EX9E; skips the next instruction if key stored in V[X] is pressed
                break;

                case (0x0001): //EXA1; skips the next instruction if key stored in V[X] is not pressed
                break;
            }
        break;

        case (0xF000):
            switch (opcode & 0x00FF){
                case (0x0007): //FX07; sets V[X] to the value of the dely timer
                break;

                case (0x000A): //FX0A; keypress is awaited (halt) and then stored in V[X]
                break;

                case (0x0015): //FX15; sets delay timer to V[X]
                break;

                case (0x0018): //FX18; set sound timer to V[X]
                break;

                case (0x001E): //FX1E; adds V[X] to I
                break;

                case (0x0029): //FX29; sets I to the sprite at V[X]
                break;

                case (0x0033): //FX33; stores binary-coded decimal of V[X] with most significant of the the digits at I, middle digit at I+1 and lease significant at I+2
                break;

                case (0x0055): //FX55; stores V[0] through V[X] in memory starting at memory[I]. I is incremented
                break;

                case (0x0065): //FX65 fills V[0] through V[X] with values from memory starting at memory[I] I is incremented
                break;
            }
        break;
    }
}

void draw_screen(){
}

void set_up(){
    memset(memory,0,4096);
    memset(V,0,16);
    pc=0x200;
    opcode=0;
    I=0;
    sp=0;
    memset(stack,0,sizeof(stack));
    memset(gfx,0,64*32);
    int i;
    for (i=0;i<80;i++){
        memory[i]=fontset[i];
    }

    //sets up graphics
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetVideoMode(640,320,32,SDL_HWSURFACE | SDL_DOUBLEBUF);
}

void load_game(char *game){
    FILE *file;
    file=fopen(game,"rb");
    fread(memory+0x200,1,sizeof(memory)-0x200,file);
    fclose(file);
}

void run(){
    SDL_Event event;
    while (1){
        if (SDL_PollEvent(&event)) continue;
        execute_cycle();
        draw_screen();

        //quits on esc
        Uint8 *keys=SDL_GetKeyState(NULL);
        if (keys[SDLK_ESCAPE]) exit(1);
    }
    
}

int main(){
    set_up();
    load_game("demo.c8");
    run();
}
