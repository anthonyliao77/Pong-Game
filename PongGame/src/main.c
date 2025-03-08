// GPIO Port D
#define GPIO_D_BASE 0x40020C00
#define GPIO_D_MODER ((volatile unsigned int *)(GPIO_D_BASE))  
#define GPIO_D_OTYPER ((volatile unsigned short *)(GPIO_D_BASE+0x4))  
#define GPIO_D_OSPEEDR ((volatile unsigned int *)(GPIO_D_BASE+0x8))  
#define GPIO_D_PUPDR ((volatile unsigned int *)(GPIO_D_BASE+0xC))  
#define GPIO_D_IDRLOW ((volatile unsigned char *)(GPIO_D_BASE+0x10))  
#define GPIO_D_IDRHIGH ((volatile unsigned char *)(GPIO_D_BASE+0x11))  
#define GPIO_D_ODRLOW ((volatile unsigned char *)(GPIO_D_BASE+0x14))  
#define GPIO_D_ODRHIGH ((volatile unsigned char *)(GPIO_D_BASE+0x15))  

// GPIO Port E
#define GPIO_E_BASE 0x40021000
#define GPIO_E_MODER ((volatile unsigned int *)(GPIO_E_BASE))  
#define GPIO_E_OTYPER ((volatile unsigned short *)(GPIO_E_BASE+0x4))  
#define GPIO_E_OSPEEDR ((volatile unsigned int *)(GPIO_E_BASE+0x8))  
#define GPIO_E_PUPDR ((volatile unsigned int *)(GPIO_E_BASE+0xC))  
#define GPIO_E_IDRHIGH ((volatile unsigned char *)(GPIO_E_BASE+0x10+1))  
#define GPIO_E_ODRLOW ((volatile unsigned char *)(GPIO_E_BASE+0x14))  
#define GPIO_E_ODRHIGH ((volatile unsigned char *)(GPIO_E_BASE+0x14+1))  

// SysTick
#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

//////////////////////////////////////////////////// Data Struct ////////////////////////////////////////////////////////////
#define MAX_POINTS 30

// point
typedef struct
{
    char x, y;
} POINT, *PPOINT;

// geometri
typedef struct
{
    int numpoints;
    int sizex, sizey;
    POINT px[ MAX_POINTS];
} GEOMETRY, *PGEOMETRY;

// objekt/boll
typedef struct tOBJ
{
    PGEOMETRY geo;
    int dirx, diry;
    int posx, posy;
    void (*draw) (struct tOBJ *);
    void (*clear) (struct tOBJ *);
    void (*move) (struct tOBJ *, struct tOBJ *);
    void (*set_speed) (struct tOBJ *, int, int);
} OBJECT, *POBJECT;

//////////////////////////////////////////////////// Graphic Display Routine ////////////////////////////////////////////

void graphic_initalize(void);
void graphic_clear_screen (void);
void graphic_pixel_set (int x, int y);
void graphic_pixel_clear (int x, int y);

__attribute__((naked))
void graphic_initalize(void)
{
    __asm volatile (" .HWORD 0xDFF0\n");
    __asm volatile (" BX LR\n");
};

__attribute__((naked))
void graphic_clear_screen(void)
{
    __asm volatile (" .HWORD 0xDFF1\n");
    __asm volatile (" BX LR\n");
};

__attribute__((naked))
void graphic_pixel_set(int x, int y)
{
    __asm volatile (" .HWORD 0xDFF2\n");
    __asm volatile (" BX LR\n");
};

__attribute__((naked))
void graphic_pixel_clear(int x, int y)
{
    __asm volatile (" .HWORD 0xDFF3\n");
};

//////////////////////////////////////////////////// Delay routine ////////////////////////////////////////////

void delay_250ns(void) {
    /* SystemCoreClock = 168000000 */
    *STK_CTRL = 0;
    *STK_LOAD = ((168 / 4) - 1);
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while ((*STK_CTRL & 0x10000) == 0);
    *STK_CTRL = 0;
}

void delay_micro(unsigned int us) {
    for (unsigned int i = 0; i < us; i++) {
        delay_250ns();
        delay_250ns();
        delay_250ns();
        delay_250ns();
    }
}

void delay_milli(unsigned int ms) {
    for (unsigned int i = 0; i < ms; i++) {
        delay_micro(1000);
    }
}

//////////////////////////////////////////////////// Keypad routine ////////////////////////////////////////////
// PORT D 8-15
// Aktivera en rad för läsning
void kbdActivate( unsigned int row )
{
  switch( row )
  {
    case 1: *GPIO_D_ODRHIGH = 0x10;   break;
    case 2: *GPIO_D_ODRHIGH = 0x20;   break;
    case 3: *GPIO_D_ODRHIGH = 0x40;   break;
    case 4: *GPIO_D_ODRHIGH = 0x80;   break;
    case 0: *GPIO_D_ODRHIGH = 0x00;   break;
  }
}

// Läs en rad och returnera vilken kolumn som är ett
// (antar endast en tangent nedtryckt)
int kbdGetCol ( void )
{
  unsigned short c;
  c = *GPIO_D_IDRHIGH;
  if ( c & 0x8 )  return 4;
  if ( c & 0x4 )  return 3;
  if ( c & 0x2 )  return 2;
  if ( c & 0x1 )  return 1;
  return 0;
}

// Returnera en byte som motsvarar den knapp som är nedtryckt,
// eller 0xFF om ingen knapp är nedtryckt
unsigned char keyb(void) {
    unsigned char key[] = {
        1, 2, 3, 0xA,
        4, 5, 6, 0xB,
        7, 8, 9, 0xC,
        0xE, 0, 0xF, 0xD
    };

    int row;
    int col;

    for (row = 1; row <= 4; row++) {
        kbdActivate(row);
        if ((col = kbdGetCol())) {
            kbdActivate(0);
            return key[4 * (row - 1) + (col - 1)];
        }
    }
    kbdActivate(0);
    return 0xFF;  
}

//////////////////////////////////////////////////// Initialise Port D ////////////////////////////////////////////////////////////////////

void init_app(void)
{
// Konfigurera D8-D15 som ingångar för keypad
    *GPIO_D_MODER &= 0xFFFF0000; // Nollställ D8-D15
    *GPIO_D_MODER |= 0x55000000; // Sätt D8-D15 som ingångar

    *GPIO_D_PUPDR &= 0xFF0000; // Nollställ pull-up/pull-down för D8-D15
    *GPIO_D_PUPDR |= 0xAAAA00; // Aktivera pull-down för D8-D15

// Pinnarna som väljer rad skall vara spänningssatta (Push/Pull)
    *GPIO_D_OTYPER= 0x00000000;
    *GPIO_D_OSPEEDR = 0x00000000;
    *GPIO_D_ODRLOW = 0;
    *GPIO_D_ODRHIGH = 0;
}

//////////////////////////////////////////////// BALL OBJECT ///////////////////////////////////////////////////////

GEOMETRY ball_geometry = {
    12, /* pixlar */
    4, 4, /* sizex, sizey */
    {
        {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {3, 1}, {3, 2}
    }
};

void draw_ballobject(POBJECT object) {
    for (int i = 0; i < object->geo->numpoints; i++) {
        graphic_pixel_set(object->posx + object->geo->px[i].x, object->posy + object->geo->px[i].y);
    }
}

void clear_ballobject(POBJECT object) {
    for (int i = 0; i < object->geo->numpoints; i++) {
        graphic_pixel_clear(object->posx + object->geo->px[i].x, object->posy + object->geo->px[i].y);
    }
}

void move_ballobject(POBJECT ball, POBJECT paddle) {
    clear_ballobject(ball);
    // kolla om bollen nuddade paddel
    // genom att jämföra bollens position med paddelns position

  
    if (ball->posx + ball->geo->sizex >= paddle->posx && ball->posx <= paddle->posx + paddle->geo->sizex) {
        if (ball->posy + ball->geo->sizey >= paddle->posy && ball->posy <= paddle->posy + paddle->geo->sizey) {
            ball->posx += 1;
            ball->dirx = -ball->dirx;
        }
    }

    ball->posx += ball->dirx;
    ball->posy += ball->diry;

    // check om boll utanför i X-led
    if (ball->posx < 1 || ball->posx + ball->geo->sizex > 128) {
        clear_ballobject(ball);
                ball->posx = 64;
                ball->posy = 32;
                ball->set_speed(ball, 6, 1); 
        draw_ballobject(ball);
        delay_milli(1);
        return;
    }

    if (ball->posy < 1) {
        ball->diry *= -1;
    }
    if (ball->posy + ball->geo->sizey > 64) {
        ball->diry *= -1;
    }

    draw_ballobject(ball);
}

void set_ballobject_speed(POBJECT object, int speedx, int speedy) {
    object->dirx = speedx;
    object->diry = speedy;
}

static OBJECT bollen = {
    &ball_geometry, /* geometri för en boll */
    6, 1, /* initiala riktningskoordinater */
    64, 32, /* initial startposition */
    draw_ballobject,
    clear_ballobject,
    move_ballobject,
    set_ballobject_speed
};

//////////////////////////////////////////////// Paddle Object ///////////////////////////////////////////////////////

// player 1 paddel
GEOMETRY paddle_geometry =
{
    27, /* pixlar */
    5,9, /* sizex, sizey */
    {
        {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8}, // tacka mig för denna fina formen
        {1,0},                                          {1,8},
        {2,0},            {2,3},{2,4},{2,5},            {2,8},
        {3,0},                                          {3,8},
        {4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8}
    }
};

// Paddel för player 2
GEOMETRY paddle_geometry1 =
{
    27, /* pixlar */
    5,9, /* sizex, sizey */
    {
        {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},
        {1,0},                                          {1,8},
        {2,0},                                          {2,8},
        {3,0},                                          {3,8},
        {4,0},{4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{4,7},{4,8}
    }
};

void set_paddle_speed(POBJECT object, int speedx, int speedy) {
    object->dirx = speedx;
    object->diry = speedy;
}


// implemenationer av paddelobjekt

void move_paddleobject(POBJECT object) {
    // clear paddel
    clear_ballobject(object);  
    
    // om paddel går utanför skärmen Y-led UPP
    if (object->posy < 0) {
        object->posy = 0;
    }
    // upd. paddel pos.
    object->posy += object->diry;

    // om paddel går utanför skärmen X-led VÄNSTER
    if (object->posx < 0) {
        object->posx = 0;
    }

    // om paddel går utanför skärmen Y-led NER
    if (object->posy + object->geo->sizey > 64) {
        object->posy = 64 - (object->geo->sizey);
    }

    // rita paddel
    draw_ballobject(object);
}

static OBJECT paddle = {
    &paddle_geometry, /* geometri för en paddel */
    0, 0, /* initiala riktningskoordinater */
    1, 32, /* initial startposition */
    draw_ballobject,
    clear_ballobject,
    move_paddleobject,
    set_paddle_speed,
};

static OBJECT paddleP2 = {
    &paddle_geometry1, /* geometri för en paddel */
    0, 0, /* initiala riktningskoordinater */
    122, 32, /* initial startposition */
    draw_ballobject,
    clear_ballobject,
    move_paddleobject,
    set_paddle_speed,
};

//////////////////////////////////////////////// Main ///////////////////////////////////////////////////////

int main(void) {
    char c;
    POBJECT p = &bollen;
    POBJECT paddle1 = &paddle;
    POBJECT paddle2 = &paddleP2;

    init_app(); // keypad port D
    graphic_initalize(); // grafisk display
    graphic_clear_screen(); // clear screen

    while (1) {
        p->move(p, paddle1); // move boll och check kollision med paddle1
        paddle1->move(paddle1, p); // move paddle och check kollision med boll

        p->move(p, paddle2); // move boll och check kollision med paddle2
        paddle2->move(paddle2, p); // move paddle och check kollision med boll

        // FPS
        delay_micro(1000/60);
        c = keyb();
        
        switch (c) {
            // player 2 tangentbord
            case 0xB: paddle2->set_speed(paddle2, 0, -6); break; // Upp
            case 0xD: paddle2->set_speed(paddle2, 0, 6); break; // Ner
            // player 1 tangentbord
            case 3: paddle1->set_speed(paddle1, 0, -6); break; // Upp
            case 9: paddle1->set_speed(paddle1, 0, 6); break; // Ner
            case 6: // starta nytt spel/ reset position för boll & ta bort gamla boll
                clear_ballobject(p);
                p->posx = 1;
                p->posy = 1;
                p->set_speed(p, 10, 1);
                break;
            default:paddle1->set_speed(paddle1, 0, 0);  // stanna paddle1
                    paddle2->set_speed(paddle2, 0, 0); // stanna paddle2
                    break; 
        }
    }
}
