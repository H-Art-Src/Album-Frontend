//Purpose: Display all Discogs titles sent from python script.
//Author: James R.


#include "../vendor/raylib/src/raylib.h"
#include "../vendor/raylib/src/raymath.h"
#include <string.h>
#include <stdio.h>
#include <limits.h>
//#include <libcurl.h>
//#include <unistd.h>


static void DrawTextBoxed(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint);   // Draw text using font inside rectangle limits
static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint);    // Draw text using font inside rectangle limits with support for text selection
static bool checkCollision(Camera camera,Vector3 origin,float buttonScaleX , float buttonScaleY, float buttonScaleZ);
static bool drawButton(char* text, Rectangle rec );

typedef struct albumEntry
{
    char *title;
    char *desc;
    char *img;
} albumEntry;

const Vector3 selectedLocation = (Vector3){ 0.0f, 3.0f, 5.0f };


int GLOBAL_HOVERING = 3;
//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int justgo(void (*functionPtr)() , struct albumEntry entries[] , int size)
{
    //window
    int screenWidth = 800;
    int screenHeight = 450;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "3DT GAME MANAGER");

    //file dir string 
    char fileDir[1024] = "";
    char* prefix = "resources/"; //ASSETS_PATH or ../../resources for windows/linux , ../resources/ for mac.
    //char cwd[96];
    //if (getcwd(cwd, sizeof(cwd)) != NULL) { printf("Current working dir: %s\n", cwd);}
    // Initialization - 3d background
    //--------------------------------------------------------------------------------------
    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 3.0f, 10.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 5.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    
    //Load textures
    strcat(fileDir, prefix);
    strcat(fileDir,"billboard.png");
    Texture2D bill = LoadTexture(fileDir);    // default texture
    strcpy(fileDir,"");
    strcat(fileDir, prefix);
    strcat(fileDir,"selected.png");    
    Texture2D billSelected = LoadTexture(fileDir);    // selected album texture
    strcpy(fileDir,"");
    strcat(fileDir, prefix);
    strcat(fileDir,"centering.png");    
    Texture2D billCenterer = LoadTexture(fileDir);    // selected album texture
    strcpy(fileDir,"");

    // Entire billboard texture, source is used to take a segment from a larger texture.
    Rectangle source = { 0.0f, 0.0f, (float)bill.width, (float)bill.height };
    // NOTE: Billboard locked on axis-Y
    Vector3 billUp = { 0.0f, 1.0f, 0.0f };
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Initialization - foreground
    //--------------------------------------------------------------------------------------

    const char text[] = "Text cannot escape\tthis container\t...word wrap also works when active so here's \
a long text for testing.\n\nLorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod \
tempor incididunt ut labore et dolore magna aliqua. Nec ullamcorper sit amet risus nullam eget felis eget.";

    bool resizing = false;
    bool wordWrap = true;

    Rectangle container = { screenWidth/2, 25.0f, screenWidth/2 - 50.0f, screenHeight - 250.0f };
    Rectangle resizer = { container.x + container.width - 17, container.y + container.height - 17, 14, 14 };

    // Minimum width and heigh for the container rectangle
    const float minWidth = 60;
    const float minHeight = 60;
    float maxWidth = screenWidth - 50.0f;
    float maxHeight = screenHeight - 160.0f;

    Vector2 lastMouse = { 0.0f, 0.0f }; // Stores last mouse coordinates
    Color borderColor = MAROON;         // Container border color
    Font font = GetFontDefault();       // Get default system font

    //GENERAL DATA
    int gmLibSize = 0;
    int gmSelect = -1;
    int gmLastSelect = -1;
    float gmScroll = 0.0f;
    float gmScrollMin = -5.0f;
    float gmScrollMax = gmLibSize*1.0f +2.5f;
    float gmSeperationScale = 1.0f;
    bool gmShortMode = false;
    //ANIMATION DATA 
    float scrollSmootherBank = 0.0f;
    float selectionChangeLerp = 0.0f;
    float currentScrollSpeed = 0.19f;
    bool finishedSelectionChange = true;
    Vector3 lastSelectionLocation = (Vector3){ 0.0f, 0.0f, 0.0f }; //doesn't matter what initial value it is becuase of TEMP.
    //TEXTURE DATA
    Texture2D gmCovers[1024];

    for(int i = 0 ; i < size ; i++)
    {
        gmCovers[i] = LoadTexture(entries[i].img);
        gmLibSize++;
    }

    //DEFAULT EXPAND MODE OPTIONS
    gmScroll = 0.0f;
    gmScrollMin = -10.0f;
    gmScrollMax = gmLibSize*1.0f + 5.0f;
    gmSeperationScale = 1.0f;

    // Main loop
    while (!WindowShouldClose())
    {   
        //inputs
        //----------------------------------------------------------------------------------
        bool inputM1 = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        bool kUp = IsKeyDown(KEY_UP);
        bool kDown = IsKeyDown(KEY_DOWN);
        bool kLeft = IsKeyPressed(KEY_LEFT);
        bool kRight = IsKeyPressed(KEY_RIGHT);
        bool kEnter = IsKeyDown(KEY_ENTER);
        Vector2 scrollVector = GetMouseWheelMoveV();
        //Math
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        Vector2 screenScale = (Vector2){(float)screenWidth/800.0f,(float)screenHeight/450.0f};
        // Draw 3D
        //----------------------------------------------------------------------------------
        BeginDrawing();
        //UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
        DrawGrid(10, 1.0f);        // Draw a grid
        //Drawing the listings and collision.
        int selectMade = -1;
        float scrollDelta = -scrollVector.y - kUp*.25f + kDown*.25f;

        //Scroll Animation
        if (scrollVector.y != 0.0f || kUp || kUp) currentScrollSpeed = 0.19f; // we want to return the original speed if nos crolling is happening
        scrollSmootherBank += (scrollSmootherBank > 1.0f && scrollDelta > 0.0f) ? 0.0f : ( (scrollSmootherBank < -1.0f  && scrollDelta < 0.0f) ? 0.0f : scrollDelta);
        scrollDelta = scrollSmootherBank*currentScrollSpeed;
        scrollSmootherBank -= scrollSmootherBank*currentScrollSpeed;
        selectionChangeLerp = (selectionChangeLerp + .05 > 1.0f)? 1.0f : selectionChangeLerp + .05f; //animation

        //show scroll centering
        if (selectionChangeLerp > .99f) 
        {
            float fadeDivide = .005;
            float alpha = (scrollSmootherBank > 0.0f)? (int)(scrollSmootherBank/fadeDivide) : (int)(scrollSmootherBank/-fadeDivide);
            DrawBillboard(camera, billCenterer, (Vector3){ -6.0f, 2.5f, 2.0f } , 2.0f, (Color){255,255,255,(alpha>255)? 255: alpha});
        }
        gmScroll = (scrollDelta < 0)? (( (gmScroll + scrollDelta) <= gmScrollMin )? gmScrollMin : gmScroll + scrollDelta) : ( ((gmScroll + scrollDelta) >= gmScrollMax)? gmScrollMax : gmScroll + scrollDelta); // just cannot pass scroll max and scrollmin
        Vector3 lastSelectionLocationTEMP = (Vector3){ -10.0f, -10.0f, 0.0f };
        bool reCenterSelect = false;
        for (int i=0; i < gmLibSize; i++)
        {
            //pick board position and size
            Texture2D billCover = gmCovers[i%5];
            billCover.width = 256;
            billCover.height = 256;
            Vector3 titleCardOrigin = (Vector3){-5.0f , 5.0f + gmScroll*2.0f + ((float)i) * 3.0f * gmSeperationScale - (float)gmLibSize*3.0f , 0.0f };
            float bbSize = 2.0f;
            if(gmShortMode)
            {
                bbSize = 0.2f;
            }
            else if(titleCardOrigin.y < -10.0f)
            {
                titleCardOrigin = (Vector3){5.0f + -gmScroll*0.1f + ((float)(gmLibSize-i))*.05f , -1.5f , -1.0f };
            }
            else if(titleCardOrigin.y < 0.0f)
            {
                titleCardOrigin = Vector3Lerp( (Vector3){-5.0f  , -0.1f , 0.0f} , (Vector3){5.0f  , -1.0f , -1.0f} , titleCardOrigin.y/-10.0f ) ;
            }
            else if(titleCardOrigin.y > 5.0f)
            {
                titleCardOrigin = (Vector3){-gmScroll*0.1f + ((float)(gmLibSize-i))*.05f - 5.0f , 5.0f , 0.01f };
            }
            else if(kEnter && !reCenterSelect && selectionChangeLerp > .99f && titleCardOrigin.y < 3.0f) //select a choice in the left side using enter key rather than mouse
            {
                if (gmSelect == i) 
                {
                    reCenterSelect = true;
                    scrollSmootherBank = ((-2.5f - ((float)i) * 3.0f * gmSeperationScale + (float)gmLibSize*3.0f)/2) - gmScroll;
                }
                else selectMade = i;
            }
            //draw correct cover based on select status
            if(i == gmSelect)
            {
                if(!gmShortMode) DrawBillboard(camera, billCover, Vector3Lerp(titleCardOrigin , selectedLocation, selectionChangeLerp) , 2.0f, WHITE);
                else DrawBillboard(camera, billCover, selectedLocation , 2.0f, WHITE);
                DrawBillboard(camera, billSelected, Vector3Lerp(  lastSelectionLocation , titleCardOrigin , selectionChangeLerp) , 2.0f, WHITE);//bill selected image is always 2.0f
                lastSelectionLocationTEMP = titleCardOrigin;
            }
            else if(i == gmLastSelect)
            {
                if(!gmShortMode) DrawBillboard(camera, billCover, Vector3Lerp( selectedLocation , titleCardOrigin , selectionChangeLerp)  , bbSize, WHITE);
                //lastSelectionLocation = titleCardOrigin; //Optional animation option: have the selector A for the lerp always be it's current location.
            }
            else if(!gmShortMode)
            {
                DrawBillboard(camera, billCover, titleCardOrigin , bbSize, WHITE);
            }
            //select title and check
            if (inputM1 && checkCollision(camera , titleCardOrigin , gmShortMode ? 4.0f : bbSize/2.0f , gmShortMode ? bbSize/0.5f : bbSize/2.0f , bbSize/2.0f ))//todo z/y aspect should be different for short mode?
            {
                if(i == gmSelect)
                {
                    /*
                    lastSelectionLocation = (Vector3){GetRandomValue(-1,1),GetRandomValue(-1,1),GetRandomValue(-1,1)};
                    selectionChangeLerp = 0.8f;
                    gmLastSelect = gmSelect;*/ //optional animation: You already selected this!
                    scrollSmootherBank = ((-2.5f - ((float)i) * 3.0f * gmSeperationScale + (float)gmLibSize*3.0f)/2) - gmScroll;
                    reCenterSelect = true;
                    //gmScroll = (-2.5f - ((float)i) * 3.0f * gmSeperationScale + (float)gmLibSize*3.0f)/2;
                }
                else// selecting
                {
                    selectMade = i;
                }
            }
        }
        if(kLeft) selectMade = gmSelect + 1;
        if(kRight) selectMade = gmSelect - 1;
        if(selectMade >= 0 && selectMade >= 0 && selectMade < gmLibSize)
        {
            gmLastSelect = gmSelect;
            gmSelect = selectMade;
            selectionChangeLerp = 0.0f;
            lastSelectionLocation = lastSelectionLocationTEMP;
            if(gmShortMode)scrollSmootherBank = ((-2.5f - ((float)gmSelect) * 3.0f * gmSeperationScale + (float)gmLibSize*3.0f)/2) - gmScroll;
            else finishedSelectionChange = false;
            currentScrollSpeed = .025f;
        }
        //auto scroll to location when finishing animation
        else if (!finishedSelectionChange && selectionChangeLerp > .99f)
        {
            scrollSmootherBank = ((-2.5f - ((float)gmSelect) * 3.0f * gmSeperationScale + (float)gmLibSize*3.0f)/2) - gmScroll;
            finishedSelectionChange = true;
        }

        EndMode3D();
        //END 3D
        //----------------------------------------------------------------------------------

        // Draw 2D
        //----------------------------------------------------------------------------------
        container = (Rectangle){ screenWidth/2 + 32 , screenHeight/2 + 32 , container.width, container.height};
        DrawRectangleLinesEx(container, 2, borderColor);    // Draw container border
        for (int i=0; i < gmLibSize; i++) //2d elements for the list
        {
            Vector3 titleCardOrigin = (Vector3){-5.0f , 5.0f + gmScroll*2.0f  + ((float)i) * 3.0f * gmSeperationScale - (float)gmLibSize*3.0f  , 0.0f };
            Vector2 w2s = GetWorldToScreen(titleCardOrigin , camera);
            if (gmShortMode) {DrawText(entries[i].title,  w2s.x , w2s.y - 8 , (int)16.0f * screenScale.y, ORANGE);}
            else
            {
                if(titleCardOrigin.y > 5.0f) {titleCardOrigin.x = -titleCardOrigin.y; titleCardOrigin.y = 5.0f; w2s = GetWorldToScreen(titleCardOrigin , camera);}
                DrawText(entries[i].title,  w2s.x - (int)(42.0f * screenScale.x) , w2s.y + (int)(64.0f * screenScale.y) , (int)(16.0f * screenScale.y), ORANGE);
            }
        }
        if (gmSelect>=0)
        {
            DrawTextBoxed(font, entries[gmSelect].desc, (Rectangle){ container.x + 4 ,  container.y , container.width - 4, container.height - 4 }, 20.0f, 2.0f, wordWrap, GRAY);
        }
        else
        {
            DrawTextBoxed(font, text, (Rectangle){ container.x + 4 ,  container.y , container.width - 4, container.height - 4 }, 20.0f, 2.0f, wordWrap, GRAY);
        }
        DrawRectangleRec((Rectangle){ container.x + container.width - 17 , container.y + container.height - 17, 15, 15 }, borderColor);     // Draw the resize box
        //----------------------------------------------------------------------------------
        //INSTALL AND PLAY
        //---------------------------------------------------------------------------------- 
        // if (drawButton("+INSTALL+",(Rectangle){ container.x , container.y + container.height + 8 , 180, 32 }) )
        // {
        //     functionPtr();
        // }
        //----------------------------------------------------------------------------------
        //TOGGLE SHORT MODE
        //----------------------------------------------------------------------------------   
        if (drawButton((gmShortMode)? "expanD" : "SHORTEn",(Rectangle){ 0 , screenHeight - 32, 160, 32 }) )
        {
            gmShortMode = !gmShortMode;
            if (gmShortMode)
            {//5.0f + gmScroll*2.0f  + ((float)i) * 3.0f * gmSeperationScale - (float)gmLibSize*3.0f
                gmSeperationScale = 0.22f;
                gmScrollMin =   gmLibSize*1.5f*(1.0f - gmSeperationScale) - 2.0f;
                gmScrollMax = gmLibSize*1.5f - 1.0f;
                gmScroll = gmScrollMin;
                scrollSmootherBank = ((-2.5f - ((float)gmSelect) * 3.0f * gmSeperationScale + (float)gmLibSize*3.0f)/2) - gmScroll;//gmScrollMax;
            }
            else
            {
                //gmScroll = 0.0f;
                gmScrollMin = -10.0f;
                gmScrollMax = gmLibSize*1.0f + 5.0f;
                gmSeperationScale = 1.0f;
            }
        }
        //----------------------------------------------------------------------------------
        //DISTRO STATUS AND WELCOME TEXT.
        //----------------------------------------------------------------------------------
        // DrawRectangleRec((Rectangle){0,0,screenWidth,screenScale.y*50.0f} , (Color){50,50,50,150});
        // DrawText("DistroSite.com | Relevent Distro news",  (int)64.0f*screenScale.x , (int)16.0f*screenScale.y , (int)16.0f*screenScale.y , ORANGE);
        // DrawCircle((int)64.0f*screenScale.x -16 , (int)24.0f*screenScale.y, 12.0f, (Color){0,150,0,150});
        // DrawCircle((int)64.0f*screenScale.x -16 , (int)24.0f*screenScale.y, 8.0f, GREEN);
        //----------------------------------------------------------------------------------
        // Update dragable container
        //----------------------------------------------------------------------------------
        maxWidth = GetScreenWidth()/2 - 50.0f;
        maxHeight = GetScreenHeight()/2 - 100.0f;
        if (IsKeyPressed(KEY_SPACE)) wordWrap = !wordWrap;
        Vector2 mouse = GetMousePosition();
        if (CheckCollisionPointRec(mouse, container))
        {
            borderColor = Fade(MAROON, 0.4f);
        }
        else if (!resizing) borderColor = MAROON;
        if (resizing)
        {
            //resize for mouse and window screen
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) resizing = false;
            float width = container.width + (mouse.x - lastMouse.x);
            container.width = (width > minWidth)? ((width < maxWidth)? width : maxWidth) : minWidth;
            float height = container.height + (mouse.y - lastMouse.y);
            container.height = (height > minHeight)? ((height < maxHeight)? height : maxHeight) : minHeight;
        }
        else
        {
            //only resize for the window screen
            if (CheckCollisionPointRec(mouse, resizer))
            {
                if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) resizing = true;
                GLOBAL_HOVERING = 2;
            }
            float width = container.width;
            container.width = (width > minWidth)? ((width < maxWidth)? width : maxWidth) : minWidth;
            float height = container.height;
            container.height = (height > minHeight)? ((height < maxHeight)? height : maxHeight) : minHeight;
        }
        resizer.x = container.x + container.width - 17;
        resizer.y = container.y + container.height - 17;
        lastMouse = mouse; // Update mouse
        //----------------------------------------------------------------------------------
        if(GLOBAL_HOVERING)GLOBAL_HOVERING--;//timeout the button hovering to prevent collision
        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(bill);        // Unload texture
    UnloadTexture(billSelected);
    UnloadTexture(billCenterer);
    for(int i=0; i < gmLibSize; i++) UnloadTexture(gmCovers[i]);//sizeof(gmCovers);
    CloseWindow();              // Close window and OpenGL context
    //free up memory
    //--------------------------------------------------------------------------------------

    return 0;
}
//--------------------------------------------------------------------------------------
// button boolean. Draw a button and return true if clicked.
//--------------------------------------------------------------------------------------
static bool drawButton(char* text, Rectangle rec ){
    if(CheckCollisionPointRec(GetMousePosition(), rec))
    {
        GLOBAL_HOVERING = 2;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            DrawRectangleRec((Rectangle){rec.x , rec.y - 4 , rec.width + 4 , rec.height + 4}, (Color){0,0,100,255});     // Draw install/play box
            DrawText(text, rec.x + 4 , rec.y - 1 , rec.height , BLUE );
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                return true;
            }
            return false;
        }
        DrawRectangleRec((Rectangle){rec.x , rec.y - 4 , rec.width + 4 , rec.height + 4}, (Color){0,0,100,255});     // Draw install/play box
        DrawRectangleRec((Rectangle){rec.x + 4 , rec.y - 4  ,rec.width , rec.height} , (Color){0,0,200,255});     // Draw install/play box
        DrawText(text, rec.x + 4 , rec.y - 1 , rec.height , WHITE );
        return false;
    }
    DrawRectangleRec(rec, (Color){0,0,200,155});     // Draw install/play box
    DrawText(text, rec.x + 1 , rec.y , rec.height , GREEN);
    return false;
}

//--------------------------------------------------------------------------------------
// collision
//--------------------------------------------------------------------------------------
static bool checkCollision(Camera camera,Vector3 origin,float buttonScaleX, float buttonScaleY, float buttonScaleZ)
{
    if (GLOBAL_HOVERING) return false;//always false if hovering over a 2D button.
    Ray ray = GetMouseRay(GetMousePosition(), camera);
    RayCollision collision = GetRayCollisionBox(ray,
    (BoundingBox){(Vector3){ origin.x - buttonScaleX, origin.y - buttonScaleY, origin.z - buttonScaleZ },
    (Vector3){ origin.x + buttonScaleX, origin.y + buttonScaleY, origin.z + buttonScaleZ }});
    return collision.hit;
}
//--------------------------------------------------------------------------------------
// Module functions definition for text box
//--------------------------------------------------------------------------------------

// Draw text using font inside rectangle limits
static void DrawTextBoxed(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint)
{
    DrawTextBoxedSelectable(font, text, rec, fontSize, spacing, wordWrap, tint, 0, 0, WHITE, WHITE);
}

// Draw text using font inside rectangle limits with support for text selection
static void DrawTextBoxedSelectable(Font font, const char *text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart, int selectLength, Color selectTint, Color selectBackTint)
{
    int length = TextLength(text);  // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0;          // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/(float)font.baseSize;     // Character rectangle scaling factor

    // Word/character wrapping mechanism variables
    enum { MEASURE_STATE = 0, DRAW_STATE = 1 };
    int state = wordWrap? MEASURE_STATE : DRAW_STATE;

    int startLine = -1;         // Index where to begin drawing (where a line begins)
    int endLine = -1;           // Index where to stop drawing (where a line ends)
    int lastk = -1;             // Holds last value of the character position

    for (int i = 0, k = 0; i < length; i++, k++)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;
        i += (codepointByteCount - 1);

        float glyphWidth = 0;
        if (codepoint != '\n')
        {
            glyphWidth = (font.glyphs[index].advanceX == 0) ? font.recs[index].width*scaleFactor : font.glyphs[index].advanceX*scaleFactor;

            if (i + 1 < length) glyphWidth = glyphWidth + spacing;
        }

        // NOTE: When wordWrap is ON we first measure how much of the text we can draw before going outside of the rec container
        // We store this info in startLine and endLine, then we change states, draw the text between those two variables
        // and change states again and again recursively until the end of the text (or until we get outside of the container).
        // When wordWrap is OFF we don't need the measure state so we go to the drawing state immediately
        // and begin drawing on the next line before we can get outside the container.
        if (state == MEASURE_STATE)
        {
            // TODO: There are multiple types of spaces in UNICODE, maybe it's a good idea to add support for more
            // Ref: http://jkorpela.fi/chars/spaces.html
            if ((codepoint == ' ') || (codepoint == '\t') || (codepoint == '\n')) endLine = i;

            if ((textOffsetX + glyphWidth) > rec.width)
            {
                endLine = (endLine < 1)? i : endLine;
                if (i == endLine) endLine -= codepointByteCount;
                if ((startLine + codepointByteCount) == endLine) endLine = (i - codepointByteCount);

                state = !state;
            }
            else if ((i + 1) == length)
            {
                endLine = i;
                state = !state;
            }
            else if (codepoint == '\n') state = !state;

            if (state == DRAW_STATE)
            {
                textOffsetX = 0;
                i = startLine;
                glyphWidth = 0;

                // Save character position when we switch states
                int tmp = lastk;
                lastk = k - 1;
                k = tmp;
            }
        }
        else
        {
            if (codepoint == '\n')
            {
                if (!wordWrap)
                {
                    textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }
            }
            else
            {
                if (!wordWrap && ((textOffsetX + glyphWidth) > rec.width))
                {
                    textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                    textOffsetX = 0;
                }

                // When text overflows rectangle height limit, just stop drawing
                if ((textOffsetY + font.baseSize*scaleFactor) > rec.height) break;

                // Draw selection background
                bool isGlyphSelected = false;
                if ((selectStart >= 0) && (k >= selectStart) && (k < (selectStart + selectLength)))
                {
                    DrawRectangleRec((Rectangle){ rec.x + textOffsetX - 1, rec.y + textOffsetY, glyphWidth, (float)font.baseSize*scaleFactor }, selectBackTint);
                    isGlyphSelected = true;
                }

                // Draw current character glyph
                if ((codepoint != ' ') && (codepoint != '\t'))
                {
                    DrawTextCodepoint(font, codepoint, (Vector2){ rec.x + textOffsetX, rec.y + textOffsetY }, fontSize, isGlyphSelected? selectTint : tint);
                }
            }

            if (wordWrap && (i == endLine))
            {
                textOffsetY += (font.baseSize + font.baseSize/2)*scaleFactor;
                textOffsetX = 0;
                startLine = endLine;
                endLine = -1;
                glyphWidth = 0;
                selectStart += lastk - k;
                k = lastk;

                state = !state;
            }
        }

        if ((textOffsetX != 0) || (codepoint != ' ')) textOffsetX += glyphWidth;  // avoid leading spaces
    }
}
