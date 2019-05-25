#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#define CARRY 0x000F
#define SHIFT1(opcode) ((opcode & 0x00F0) >> 4)
#define SHIFT2(opcode) ((opcode & 0x0F00) >> 8)
#define SHIFT3(opcode) ((opcode & 0xF000) >> 12)

unsigned char delay_timer,sound_timer;
unsigned char memory[4096];
unsigned char V[16];
unsigned char fontset[]={
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
unsigned char gfx[64*32];
unsigned short I,pc,sp,opcode;
unsigned short stack[16];
int draw=0;
int wait=0;
static int key_map[]={
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f,
};

void execute_cycle(){
    int i,x,y,j,k;
    unsigned h,p;
    Uint8 *keys;

    opcode=( (memory[pc] << 8 ) | memory[pc+1]);

    printf("Executing %04X at %04X , I:%02X SP:%02X\n",opcode,pc,I,sp);

    switch (opcode & 0xF000){
        case (0x000):
            switch (opcode & 0x000F){
                case (0x0000): //00E0; clear screen
                    for (i=0;i<sizeof(gfx);i++)
                        gfx[i]=0;
                    draw=1;
                    pc+=2;
                break;

                case (0x000E): //00EE; return from subroutine
                    sp--;
                    pc=stack[sp];
                    pc+=2;
                break;

                default: //0NNN;
                break;
            }
        break;

        case (0x1000): //1NNN; jump to address at NNN
            pc=opcode & 0x0FFF;
        break;

        case (0x2000): //2NNN; call subroutine at NNN
            //put current location on stack so we can return after subroutine
            stack[sp]=pc;
            sp++;
            //go to subroutine
            pc=memory[ opcode & 0x0FFF ];
        break;

        case (0x3000): //3XNN; skip the next instruction if V[X] == NN
            if ( V[ SHIFT2(opcode) ] == ( opcode & 0x00FF ))
                pc+=2;
            pc+=2;
        break;

        case (0x4000): //4XNN; skip the next instruction if V[X] != NN
            if ( V[ SHIFT2(opcode) ] != ( opcode & 0x00FF ))
                pc+=2;
            pc+=2;
        break;

        case (0x5000): //5XY0; skip the next instruction if V[X] == V[Y]
            if ( V[ SHIFT2(opcode) ] == V[ SHIFT1(opcode) ] )
                pc+=2;
            pc+=2;
        break;

        case (0x6000): //6XNN; sets V[X] to NN
            V[ SHIFT2(opcode) ] = (opcode & 0x00FF);
            pc+=2;
        break;

        case (0x7000): //7XNN; adds NN to V[X] without changing the carry flag
            V[ SHIFT2(opcode) ] += (opcode & 0x00FF);
            pc+=2;
        break;

        case (0x8000):
            switch (opcode & 0x000F){
                case (0x0000): //8XY0; set V[X] to V[Y]
                    V[ SHIFT2(opcode) ] = V[ SHIFT1(opcode) ];
                    pc+=2;
                break;

                case (0x0001): //8XY1; set V[X] to V[X] OR V[Y]
                    V[ SHIFT2(opcode) ] = ( V[ SHIFT2(opcode) ] | V[ SHIFT1(opcode) ] );
                    pc+=2;
                break;

                case (0x0002): //8XY2; set V[X] to V[X] AND V[Y]
                    V[ SHIFT2(opcode) ] = ( V[ SHIFT2(opcode) ] & V[ SHIFT1(opcode) ] );
                    pc+=2;
                break;

                case (0x0003): //8XY3; set V[X] to V[X] XOR V[Y]
                    V[ SHIFT2(opcode) ] = ( V[ SHIFT2(opcode) ] ^ V[ SHIFT1(opcode) ] );
                    pc+=2;
                break;

                case (0x0004): //8XY4; adds V[X] to V[Y] changes the carry flag if needed
                    if ( V[ SHIFT2(opcode) ] + V[ SHIFT1(opcode) ] < 256 ){
                        V[CARRY]=0;
                    }
                    else{
                        V[CARRY]=1;
                    }
                    V[ SHIFT1(opcode) ] += V[ SHIFT2(opcode) ];
                    pc+=2;
                break;

                case (0x0005): //8XY5; V[Y] is subtracted from V[X] and carry is set to 0 if there is a borrow
                    if ( V[ SHIFT2(opcode) ] - V[ SHIFT1(opcode) ] >= 0 ){
                        V[ CARRY ] = 1;
                    }
                    else{
                        V[ CARRY ] = 0;
                    }
                    V[ SHIFT2(opcode) ] -= V[ SHIFT1(opcode) ];
                    pc+=2;
                break;

                case (0x0006): //8XY6; shifts V[X] right one and stores in V[X]. Carry flag is set to LSB of V[X] from before the shift
                    V[ CARRY ] = V[ SHIFT2(opcode) ] & 7;
                    V[ SHIFT2(opcode) ] = V[ SHIFT2(opcode) ] >> 1;
                    pc+=2;
                break;

                case (0x0007): //8XY7; sets V[X] to V[Y]-V[X]. Carry is set to zero if there is a borrow
                    if ( V[ SHIFT2(opcode) ] - V[ SHIFT1(opcode) ] >= 0 ){
                        V[ CARRY ] = 0;
                    }
                    else{
                        V[ CARRY ] = 1;
                    }
                    V[ SHIFT2(opcode) ] = V[ SHIFT1(opcode) ] - V[ SHIFT2(opcode) ];
                    pc+=2;
                break;

                case (0x000E): //8XYE; shifts V[X] left one and stores in V[X]. Carry flag is set to LSB of V[X] from before the shift
                    V[ CARRY ] = V[ SHIFT2(opcode) ] >> 7;
                    V[ SHIFT2(opcode) ] = V[ SHIFT2(opcode) ] << 1;
                    pc+=2;
                break;
            }
        break;

        case (0x9000): //9XY0; skips the next instruction if V[X] != V[Y]
            if ( V[ SHIFT2(opcode) ] != V[ SHIFT1(opcode) ] )
                pc+=2;
            pc+=2;
        break;

        case (0xA000): //ANNN; sets I to NNN
            I = (opcode & 0x0FFF);
            pc+=2;
        break;

        case (0xB000): //BNNN; jumps to NNN + V[0]
            pc = ( ( opcode & 0x0FFF ) + V[0] );
        break;

        case (0xC000): //CXNN; sets V[X] to random number < 256 AND NN
            V[ SHIFT2(opcode) ] = ( rand() % 257 ) & ( opcode & 0x00FF);
            pc+=2;
        break;

        case (0xD000): //DXYN; draws sprite at (V[X],V[Y]) with a width of 8 and a height of N
            j=V[ SHIFT2(opcode) ];
            k=V[ SHIFT1(opcode) ];
            h=opcode & 0x000F;
            V[0xF]&=0;

            for (y=0;y<h;y++){
                p=memory[I+y];
                for (x=0;x<8;x++){
                    if (p & (0x80 >> x)){
                        if (gfx[ x + j + (y+k) *64 ]){
                            V[0xF] = 1;
                        }
                        gfx[ x + j + (y+k) *64 ] ^=1;
                    }
                }
            }
            pc+=2;
        break;

        case (0xE000):
            switch (opcode & 0x000F){
                case (0x000E): //EX9E; skips the next instruction if key stored in V[X] is pressed
                    keys = SDL_GetKeyState(NULL);
                    if (keys[ key_map[ (int)V[ SHIFT2(opcode) ] ] ])
                        pc+=2;
                    pc+=2;
                break;

                case (0x0001): //EXA1; skips the next instruction if key stored in V[X] is not pressed
                    keys = SDL_GetKeyState(NULL);
                    if (!keys[ key_map[ (int)V[ SHIFT2(opcode) ] ] ])
                        pc+=2;
                    pc+=2;
                break;
            }
        break;

        case (0xF000):
            switch (opcode & 0x00FF){
                case (0x0007): //FX07; sets V[X] to the value of the dely timer
                    V[ SHIFT2(opcode) ] = delay_timer;
                    pc+=2;
                break;

                case (0x000A): //FX0A; keypress is awaited (halt) and then stored in V[X]
                break;

                case (0x0015): //FX15; sets delay timer to V[X]
                    delay_timer = V[ SHIFT2(opcode) ];
                    pc+=2;
                break;

                case (0x0018): //FX18; set sound timer to V[X]
                    sound_timer = V[ SHIFT2(opcode) ];
                    pc+=2;
                break;

                case (0x001E): //FX1E; adds V[X] to I
                    I+=V[ SHIFT2(opcode) ];
                    pc+=2;
                break;

                case (0x0029): //FX29; sets I to the sprite at V[X]
                    I = V[ SHIFT2(opcode) ] * 5;
                    pc+=2;
                break;

                case (0x0033): //FX33; stores binary-coded decimal of V[X] with most significant of the the digits at I, middle digit at I+1 and lease significant at I+2
                    memory[I] = V[ SHIFT2(opcode) ] / 100;
                    memory[I+1] = (V[ SHIFT2(opcode) ]/10) % 10;
                    memory[I+2] = V[ SHIFT2(opcode) ] % 10;
                break;

                case (0x0055): //FX55; stores V[0] through V[X] in memory starting at memory[I]. I is incremented
                    for (i=0;i<=SHIFT2(opcode);i++){
                        memory[I+i] = V[i];
                    }
                    pc+=2;
                break;

                case (0x0065): //FX65 fills V[0] through V[X] with values from memory starting at memory[I] I is incremented
                    for (i=0;i<SHIFT2(opcode);i++){
                        V[i] = memory[I+i];
                    }
                    pc+=2;
                break;
            }
        break;
    }

    if (delay_timer > 0)
        delay_timer--;
    if (sound_timer > 0)
        sound_timer--;
}

void draw_screen(){
    int i,j;
    SDL_Surface *surface = SDL_GetVideoSurface();
    SDL_LockSurface(surface);
    Uint32 *screen=surface->pixels;
    memset(screen,0,surface->w * surface->h * sizeof(Uint32));
    for (i=0;i<320;i++){
        for (j=0;j<640;j++){
            screen[j + i *surface->w] = gfx[(j/10) + (i/10) * 64] ? 0xFFFFFFFF : 0;
        }
    }
    SDL_UnlockSurface(surface);
    SDL_Flip(surface);
    SDL_Delay(15);
}

void set_up(){
    pc=0x200;
    opcode=0x200;
    sp=0;
    memset(V,0,16);
    memset(stack,0,sizeof(stack));
    memset(gfx,0,64*32);
    memset(memory,0,4096);
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
    if (!file){
        fprintf(stderr,"Could not open %s\n",game);
        exit(0);
    }
    fread(memory+0x200,1,4096-0x200,file);
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
