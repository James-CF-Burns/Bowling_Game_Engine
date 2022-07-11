#include <iostream>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifdef _WIN32
	//define something for Windows (32-bit and 64-bit, this part is common)
	#include <gl/freeglut.h>		// For windows
	#ifdef _WIN64
		//define something for Windows (64-bit only)
	#endif
#elif __APPLE__
	#include <GLUT/glut.h>			// for Mac OS
#endif
#include <stdio.h>
#include "ReadOff.h"

#define objectCount 6
using namespace std;
#define   MAXROW	400
#define   MAXCOL	400

typedef 	unsigned char	pixel;
typedef	char	name[15];
/* image buffers used for display */
name		image_file_name;
pixel	image_buf[MAXROW*MAXCOL], out_image_buf[MAXROW*MAXCOL];
pixel 	image[MAXROW][MAXCOL], out_image[MAXROW][MAXCOL] ;   /* image arrays */
pixel	p;
FILE	*inf_handle, *outf_handle;    /*file handles created at file open */
name inf_name1 = { 'j', '.', 'b', 'm', 'p'} ;
name inf_name2 = { 't', '.', 'b', 'm', 'p'};
int		charin;
int		r,c;

float xpos = 0, ypos = 2, zpos = 0, xrot = 0, yrot = 0, angle=0.0;
float lastx, lasty;
float xrotrad, yrotrad;
float velmod = 0;

float timedif = 0;
typedef struct{
float pcurpos[3];
float pprevel[3];
float pcurvel[3];
float pprevpos[3];
float pacc[3];
float pinmass[3];
float objRotX;
float objRotY;
float objRotZ;
int rotationMod;
}object;

object pins[objectCount];

float curpos[] = {0.0f,6.0f,-10.0f};
float prevel[] = {0.0,0.0,0.0};
float curvel[] = {0.0,0.0,0.0};
float prevpos[] = {40.0f,4.0f,-10.0f};
float acc[] = {0,-0.021f,0};
float pinmass[] = {0.0, 0.18, 0.0}; // y value = 0.17 for pin & ball
float yrotradians;
float xrotradians;
typedef GLfloat point2[2];
typedef GLfloat point3[3];
typedef int     Face[3];


typedef struct
{
    int nvert, nfaces;
    point3* vertices;
    Face* faces;

} Object3D;

Object3D mySlide;
Object3D mySphere;
Object3D myPin;
void Draw_Wall_One()
{
   //back
    glBegin(GL_QUADS);
        glVertex3f(-0.1, 0, 0);
        glVertex3f(-0.1, 0, 3.5);
        glVertex3f(-0.1, 10, 3.5);
        glVertex3f(-0.1, 10, 0);
    glEnd();

    //right
    glBegin(GL_QUADS);
        glVertex3f(-0.1, 10, 0);
        glVertex3f(-0.1, 10, 3.5);
        glVertex3f(0, 10, 3.5);
        glVertex3f(0, 10, 0);
    glEnd();

    //front
    glBegin(GL_QUADS);
        glVertex3f(0, 10, 0);
        glVertex3f(0, 10, 3.5);
        glVertex3f(0, 0, 3.5);
        glVertex3f(0, 0, 0);
    glEnd();

    //left
    glBegin(GL_QUADS);
        glVertex3f(0, 0, 0);
        glVertex3f(-0.1, 0, 0);
        glVertex3f(-0.1, 0, 3.5);
        glVertex3f(0, 0, 3.5);
    glEnd();

    //top
    glBegin(GL_QUADS);
        glVertex3f(0, 10, 3.5);
        glVertex3f(-0.1, 10, 3.5);
        glVertex3f(-0.1, 0, 3.5);
        glVertex3f(0, 0, 3.5);
    glEnd();

    //bot
    glBegin(GL_QUADS);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 10, 0);
        glVertex3f(-0.1, 10, 0);
        glVertex3f(-0.1, 0, 0);
    glEnd();
}

void readOFFFile(const char* file_name, Object3D* obj) // This function was created by Tim
{

    FILE* fp;
    char str[100];
    fp = fopen(file_name, "r");

    if (fp == NULL){
        printf("Error\n");
        getchar();
        exit(0);
    }
    fscanf(fp, "%s", str);
    printf("%s\n", str);

    if (strcmp(str,"OFF") != 0){
        fclose(fp);
        printf("Error\n");
        getchar();
        exit(0);

    }
    int dump;
    fscanf(fp, "%d", & obj->nvert);
    fscanf(fp, "%d", & obj->nfaces);
    fscanf(fp, "%d", &dump);

    printf("Number of vertices: %d,\nNumber of faces: %d\n", obj->nvert, obj->nfaces);

    obj->vertices = (point3*) malloc((obj->nvert) * sizeof(point3));
    int i;
    for (i=0; i<obj->nvert; i++){
        point3 vertex;
        fscanf(fp, "%f", vertex);
        fscanf(fp, "%f", vertex+1);
        fscanf(fp, "%f", vertex+2);

        (obj->vertices)[i][0] = vertex[0];
        (obj->vertices)[i][1] = vertex[1];
        (obj->vertices)[i][2] = vertex[2];
    }

    obj->faces = (Face*) malloc((obj->nfaces) * sizeof(Face));
    for (i=0; i<obj->nfaces; i++){

        Face face;
        fscanf(fp, "%d", &dump);
        fscanf(fp, "%d", face);
        fscanf(fp, "%d", face+1);
        fscanf(fp, "%d", face+2);

        (obj->faces)[i][0] = face[0];
        (obj->faces)[i][1] = face[1];
        (obj->faces)[i][2] = face[2];
    }
    fclose(fp);
}

void draw3DShape(Object3D model) // This function was created by Tim
{
    for (int i=0; i<model.nfaces; i++){

        glBegin(GL_TRIANGLES);
            Face f;
            f[0] = model.faces[i][0];
            f[1] = model.faces[i][1];
            f[2] = model.faces[i][2];

            glVertex3fv(model.vertices[f[0]]);
            glVertex3fv(model.vertices[f[1]]);
            glVertex3fv(model.vertices[f[2]]);
        glEnd();
    }
}

void drawBowlingBall() // This function was created by James
{
	glColor3f(1.0f, 0.0f, 0.0f); // sets colour to red

// Draw ball
	glTranslatef(prevpos[0], prevpos[1], prevpos[2]);
	glutSolidSphere(0.26f,40,40);

	glPopMatrix();
}
void initialPinLocation() // This function was created by Tim
{
    for(int i = 0; i<6;i++){
        pins[i].rotationMod = 1;
        pins[i].objRotX = 0;
        pins[i].objRotY = 0;
        pins[i].objRotZ = 0;

        pins[i].pcurvel[0] = 0;
        pins[i].pcurvel[1] = 0;
        pins[i].pcurvel[2] = 0;

        pins[i].pprevel[0] = 0;
        pins[i].pprevel[1] = 0;
        pins[i].pprevel[2] = 0;
    }
    //Placing pins front
    pins[0].pcurpos[0] = 0;
    pins[0].pcurpos[1] = 0.7;
    pins[0].pcurpos[2] = -40;
    pins[0].pprevpos[0]= 0;
    pins[0].pprevpos[1]= 0.7;
    pins[0].pprevpos[2]= -40;

    //second layer left to right
    pins[1].pcurpos[0] = -0.5;
    pins[1].pcurpos[1] = 0.7;
    pins[1].pcurpos[2] = -41;
    pins[1].pprevpos[0]= -0.5;
    pins[1].pprevpos[1]= 0.7;
    pins[1].pprevpos[2]= -41;

    pins[2].pcurpos[0] = 0.5;
    pins[2].pcurpos[1] = 0.7;
    pins[2].pcurpos[2] = -41;
    pins[2].pprevpos[0]= 0.5;
    pins[2].pprevpos[1]= 0.7;
    pins[2].pprevpos[2]= -41;

    //third layer
    pins[3].pcurpos[0] = -1;
    pins[3].pcurpos[1] = 0.7;
    pins[3].pcurpos[2] = -42;
    pins[3].pprevpos[0]= -1;
    pins[3].pprevpos[1]= 0.7;
    pins[3].pprevpos[2]= -42;

    pins[4].pcurpos[0] = 1;
    pins[4].pcurpos[1] = 0.7;
    pins[4].pcurpos[2] = -42;
    pins[4].pprevpos[0]= 1;
    pins[4].pprevpos[1]= 0.7;
    pins[4].pprevpos[2]= -42;

    pins[5].pcurpos[0] = 0;
    pins[5].pcurpos[1] = 0.7;
    pins[5].pcurpos[2] = -42;
    pins[5].pprevpos[0]= 0;
    pins[5].pprevpos[1]= 0.7;
    pins[5].pprevpos[2]= -42;

}
void drawPins() // This function was created by Tim
{
    for(int i = 0; i<6;i++){
        glPushMatrix();
            glColor3f(1.0,1.0,1.0);
            glTranslatef(pins[i].pprevpos[0], pins[i].pprevpos[1], pins[i].pprevpos[2]);
            glRotatef(pins[i].objRotY, 0.0f,1.0f, 0.0f); // Y rotation is the facing direction
            glRotatef(pins[i].objRotX, 1.0f, 0.0f, 0.0f);
            glRotatef(pins[i].objRotZ, 0.0f, 0.0f, 1.0f);
            draw3DShape(myPin);
        glPopMatrix();
    }
}

void Catch() // This function was created by James
{
    velmod = 3; // turns off gravity
    curpos[0] = xpos; //+(xrot/1000);
    curpos[1] = ypos;
    curpos[2] = (zpos-2);

}

void Throw() // This function was created by James
{
    velmod = 0; // turns gravity on
    prevel[1] = 0.1; // throws object slightly upward
    acc[1] = -0.02f; // helps turn gravity on
    float yrotradi = ((yrot-90)/180) *3.141592654; //convert to radians
    prevel[2] = (float)1*sin(yrotradi); //velocity based on angle user is facing 1* is the velocity multiplyer
    prevel[0] = (float)1*cos(yrotradi);
}

void enableDepth (void) // This function was created by James
{
    glEnable (GL_DEPTH_TEST); //enable the depth testing, this is needed for the holes in the bowling ball
}

void camera (void) // This function was created by James
{
    glRotatef(xrot,1.0,0.0,0.0);  //rotate the camera on the x-axis (left and right)
    glRotatef(yrot,0.0,1.0,0.0);  //rotate the camera on the y-axis (up and down)
    glTranslated(-xpos,-ypos,-zpos); //translate the screen to the position of the camera
}

void display (void) // This function was created by James and Tim
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer and the depth buffer
    glLoadIdentity();
    camera();
    enableDepth();

    //draw ground
    glColor3f(1, 0.8, 0.000);
	glBegin(GL_QUADS);
		glVertex3f(-100.0f, -0.0f, -100.0f);
		glVertex3f(-100.0f, -0.0f,  100.0f);
		glVertex3f( 100.0f, -0.0f,  100.0f);
		glVertex3f( 100.0f, -0.0f, -100.0f);
	glEnd();

        //left wall
        glColor3f(0.000, 0.749, 1.000);
        glPushMatrix();
            glTranslatef(-3,5,-45);
            glScalef(5,5,5);
            glRotatef(90,1,0,0);
            Draw_Wall_One();
        glPopMatrix();
        //right wall
        glPushMatrix();
            glTranslatef(3,5,-45);
            glScalef(5,5,5);
            glRotatef(90,1,0,0);
            Draw_Wall_One();
        glPopMatrix();
        //roof
        glPushMatrix();
        glTranslatef(-4,5,4);
            glScalef(1,1.5,1);
            glRotatef(90,0,1,0);
            glRotatef(90,1,0,0);
            Draw_Wall_One();
        glPopMatrix();
        //behind player wall
        glPushMatrix();
        glColor3f(0.0, 0.0, 0.0);
        glTranslatef(-4,5,-45);
            glScalef(1,1.5,1);
            glRotatef(90,0,1,0);
            glRotatef(90,1,0,0);
            Draw_Wall_One();
        glPopMatrix();
        //end wall
        glColor3f(0.098, 0.098, 0.439);
        glPushMatrix();
            glTranslatef(-3,5,-45);
            glScalef(5,5,5);
            glRotatef(90,0,0,1);
            glRotatef(90,1,0,0);
            Draw_Wall_One();
        glPopMatrix();

        //draws pins

        glPushMatrix();
        //glTranslatef(i*10.0,0,j * 10.0);
        glColor3f(0.0,0.0,0.0);
        drawPins();
        drawBowlingBall();
        glPopMatrix();

	glutSwapBuffers();
    angle++; //increase the angle
}

void applyRotation() // This function was created by Tim
{
    for(int j = 0; j<6; j++){

        if(pins[j].rotationMod == 0){
        float tempX = pins[j].pcurvel[0]/0.8; //angular velocity 0.8 is radius
        pins[j].objRotY += tempX * 180/ 3.14159265359; //radian conversion
        pins[j].pprevpos[1]-= 0.5;
        }

        if(pins[j].rotationMod == 1){
            if(pins[j].objRotZ < 85 && pins[j].objRotZ > -85 ){ //stops rotation through the ground
            float tempVel = (pins[j].pcurvel[0] + pins[j].pcurvel[2]) /2;
            float tempZ = tempVel/0.8;
            pins[j].objRotZ -= tempZ * 180/ 3.14159265359;
            }
            else{
                pins[j].rotationMod = 0;
            }
        }
        if(pins[j].objRotX > 360)
            pins[j].objRotX -= 360;
        if(pins[j].objRotX < -360)
            pins[j].objRotX += 360;
        if(pins[j].objRotY > 360)
            pins[j].objRotY -= 360;
        if(pins[j].objRotY < -360)
            pins[j].objRotY += 360;
        if(pins[j].objRotZ < -360)
            pins[j].objRotZ += 360;
        if(pins[j].objRotZ > 360)
            pins[j].objRotZ -= 360;
    }
}
void applyGravity() // This function was created by James
{
    if (velmod == 0) // if ball is falling
    {
        if (curvel == 0 || prevel == 0 || curpos[1] <= 0.24) // check if ball is still, if true, the ball stops falling
        {
                acc[1] = 0;
                curpos[1] = 0.24;
                prevpos[1] = 0.24;
                velmod = 2;
        }
        for (int i=0; i<3;i++)
        {
            curpos[i] = prevpos[i] + prevel[i] + (acc[i]); // The reason I am using this formula is because I think it looks more realistic
            curvel[i] = (prevel[i]+acc[i]);
        }
        if (prevel[2] > 0) // this simulates friction
        {
            prevel[2] = prevel[2] - 0.8;
        }
        if (curpos[1] <= 0.25)
        {
            curpos[1] = 0.25;
            prevel[1] = prevel[1]+0.04;
            prevel[1] = prevel[1]*-1;
            velmod = 1;
        }
    }

    if (velmod == 1) //if ball is bouncing upward
    {
        if (curvel == 0 || prevel == 0 || curpos[1] <= 0.24) // if ball is not moving or bellow the ground, set to ground level
        {
                acc[1] = 0;
                curpos[1] = 0.24;
                prevpos[1] = 0.24;
                velmod = 2;
        }
        if (prevel[1] <=0 || curvel[1] <= 0)
        {
            velmod = 0;
        }
        if (prevel[2] > 0)
        {
            prevel[2] = prevel[2] - 0.8;
        }
        for (int i=0; i<3;i++)
        {
            curpos[i] = prevpos[i] + prevel[i] + (0.5*(acc[i]-pinmass[i]));
            curvel[i] = (prevel[i]+(acc[i]-pinmass[i]))*0.99;
        }

        if (curpos[0] > 2)
        {
            curvel[0] = 0;
            prevel[0] = 0;
        }
        if (curpos[0] < -2)
        {
            curvel[0] = 0;
            prevel[0] = 0;
        }
    }

    for (int k=0; k<3;k++)
    {
        prevpos[k] = curpos[k];
        prevel[k] = curvel[k];
    }

        if (curpos[0] > 2)
        {
            curvel[0] = 0;
            prevel[0] = 0;
        }
        if (curpos[0] < -2)
        {
            curvel[0] = 0;
            prevel[0] = 0;
        }
}

void reshape (int w, int h) // This function was created by James
{
    glViewport (0, 0, (GLsizei)w, (GLsizei)h); //set the viewport to the current window size
    glMatrixMode (GL_PROJECTION);

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)w / (GLfloat)h, 1.0, 1000.0); //set the perspective (angle of sight, width, height, ,depth)
    glMatrixMode (GL_MODELVIEW);

}

void hitPins() // This function was created by James
{
    for (int i=0; i<6; i++)
    {
        for (int x=0; x<3;x++)
        {
            if (pins[i].pcurvel[1] == 0 && pins[i].pprevel[1] == 0 || pins[i].pcurpos[1] < 0.7)
            {
                pins[i].pcurpos[1] = 0.7;
                pins[i].pprevpos[1] = 0.7;
            }
            if (curpos[0] - pins[i].pcurpos[0] <= 0.24 && curpos[1] - pins[i].pcurpos[1] <= 0.24 && curpos[2] - pins[i].pcurpos[2] <= 0.24)
            {
                if (curpos[0] - pins[i].pcurpos[0] >= -1 && curpos[1] - pins[i].pcurpos[1] >= -1 && curpos[2] - pins[i].pcurpos[2] >= -1)
                {
                    pins[i].pcurvel[x] = 0.2;
                    pins[i].pprevel[x] = 0.2;
                    pins[i].pcurvel[2] = -0.2;
                }
            }
            for (int k=0; k<3;k++)
            {
                pins[i].pprevpos[k] = pins[i].pcurpos[k];
                pins[i].pprevel[k] = pins[i].pcurvel[k];
            }
            pins[i].pcurpos[x] = pins[i].pprevpos[x] + pins[i].pprevel[x] + (0.5*(acc[x]-pinmass[x]));
            pins[i].pcurvel[x] = (pins[i].pprevel[x]+(acc[x]-pinmass[x]))*0.99;
            if (pins[i].pprevel[0] > 0) // this simulates friction for the pins
            {
                pins[i].pprevel[0] = pins[i].pprevel[0] - 0.0001;
                pins[i].pcurvel[0] = pins[i].pcurvel[0] - 0.0001;
            }
            if (pins[i].pprevel[2] > 0) // this simulates friction for the pins
            {
                pins[i].pprevel[2] = pins[i].pprevel[2] - 0.001;
                pins[i].pcurvel[2] = pins[i].pcurvel[2] - 0.001;
            }
        }
        if (pins[i].pcurpos[0] > 2)
        {
            pins[i].pcurvel[0] -= 0.1;
        }
        if (pins[i].pcurpos[0] < -2)
        {
            pins[i].pcurvel[0] += 0.1;
        }

        if (pins[i].pcurpos[2] < -45)
        {
            pins[i].pcurvel[2] += 0.1;
        }
        if (pins[i].pcurpos[2] > 2)
        {
            pins[i].pcurvel[2] -= 0.8;
        }
    }
}

void timer(int) // This function was created by James
{
    hitPins();
    applyGravity();
    applyRotation();
    glutPostRedisplay();
    glutTimerFunc(6, timer, 0);
}

void	InputImage1(name inf_name) // This function was created by James
{

    if  ((inf_handle = fopen(inf_name, "rb")) == NULL)  // open failed
    {
         charin = getchar();
         exit(1);
    }

  	for ( r = 0;  r < MAXROW; r++ )
      	for ( c = 0;  c < MAXCOL; c++)  {
            if((charin=fgetc(inf_handle))==EOF)   // read failed
            {
              charin = getchar();
              exit(1);
            }
            image[r][c] = charin;
         }

   fclose(inf_handle);    // close input file
 }

 void	InputImage2(name inf_name) // This function was created by James
{

    if  ((inf_handle = fopen(inf_name, "rb")) == NULL)  // open failed
    {
         charin = getchar();
         exit(1);
    }

  	for ( r = 0;  r < MAXROW; r++ )
      	for ( c = 0;  c < MAXCOL; c++)  {
            if((charin=fgetc(inf_handle))==EOF)   // read failed
            {
              charin = getchar();
              exit(1);
            }
            out_image[r][c] = charin;
         }

   fclose(inf_handle);    // close input file
 }

void	WriteCaptions() // This function was created by James
 {
   int	i;
   char	caption1[ ] = "James Burns";
   char	caption2[ ] = "Tim Smith";

   glColor3f(1.0, 1.0, 1.0);

   glRasterPos2i(160, 30);
   for (i=0; i< sizeof(caption1); i++)
   	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, caption1[i]);

   glRasterPos2i(540, 30);
   for (i=0; i< sizeof(caption2); i++)
   	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, caption2[i]);
 }

 void	DisplayPic(void) // This function was created by James
{
   int	offset;

   offset = 0;
	for ( r = 0;  r < MAXROW; r++ )	{
      for ( c = 0;  c < MAXCOL; c++)  {
      	image_buf[MAXCOL*offset + c + 120] =  image[r][c];
      }
      offset++;
	}

   // flip image - 1st row becomes last - before calling glDrawPixels   to display processed image
   offset = 0;
	for ( r = 0;  r < MAXROW; r++ )	{
      for ( c = 0;  c < MAXCOL; c++)  {
      	out_image_buf[MAXCOL*offset + c + 120] =  out_image[r][c];
      }
      offset++;
	}

   glClear(GL_COLOR_BUFFER_BIT);

/* switch matrix mode  to 'projection' */
	glMatrixMode(GL_PROJECTION);
   glLoadIdentity();

/* set up an orthographic projection in 2D with a 760x280 viewing window  */
	gluOrtho2D(0.0,800.0, 0.0,100.0);

/* switch matrix mode back to 'model view' */
	glMatrixMode(GL_MODELVIEW);

   WriteCaptions();

/* set raster position for displaying image in graphics image buffer*/
   glRasterPos2i(90, 36);
/* load graphics image buffer with image from your own image buffer */
   glDrawPixels(MAXCOL, MAXROW, GL_LUMINANCE, GL_UNSIGNED_BYTE, image_buf); //image 1

/* now do the same for displaying processed image to right of original image*/

   glRasterPos2i(450, 36);
   glDrawPixels(MAXCOL, MAXROW, GL_LUMINANCE, GL_UNSIGNED_BYTE, out_image_buf); // image 2
   glFlush();
}

void  SetupDisplayPic(void) // This function was created by James
{
   glutInitDisplayMode(GLUT_RGB|GLUT_DEPTH);

   glutInitWindowSize(1800,1000);

	glutInitWindowPosition(0,0);

   glutCreateWindow("Image handler");
	glClearColor(0.0, 0.0, 0.0, 0.0);

   glutDisplayFunc(DisplayPic);
}

void keyboard (unsigned char key, int x, int y) // This function was created by James
{

    if (key=='w')
    {
        yrotrad = (yrot / 180 * 3.141592654f);
        xrotrad = (xrot / 180 * 3.141592654f);
        xpos += float(0.2*sin(yrotrad));
        zpos -= float(0.2*cos(yrotrad));
        //ypos -= float(sin(xrotrad)) ; // enable this to fly
        if (velmod == 3)
        {
            Catch();
        }
    }

    if (key=='s')
    {
        float xrotrad, yrotrad;
        yrotrad = (yrot / 180 * 3.141592654f);
        xrotrad = (xrot / 180 * 3.141592654f);
        xpos -= float(0.2*sin(yrotrad));
        zpos += float(0.2*cos(yrotrad)) ;
        //ypos += float(sin(xrotrad)); // enable this to fly
        if (velmod == 3)
        {
            Catch();
        }
    }

    if (key=='d')
    {
        float yrotrad;
        yrotrad = (yrot / 180 * 3.141592654f);
        xpos += float(cos(yrotrad)) * 0.2;
        zpos += float(sin(yrotrad)) * 0.2;
        if (velmod == 3)
        {
            Catch();
        }
    }

    if (key=='a')
    {
        float yrotrad;
        yrotrad = (yrot / 180 * 3.141592654f);
        xpos -= float(cos(yrotrad)) * 0.2;
        zpos -= float(sin(yrotrad)) * 0.2;
        if (velmod == 3)
        {
            Catch();
        }
    }

    if (key=='e')
        {
            Catch();
        }

    if (key=='f')
    {
        Throw();
    }
    if (key=='r')
    {
        initialPinLocation();
    }

    if (key==27)
    {
        exit(1); // press esc to exit
    }
    if (key=='p')
    {
        InputImage1(inf_name1);
        InputImage2(inf_name2);
        SetupDisplayPic();
        glutKeyboardFunc (keyboard);
    }
}

void mouseMovement(int x, int y) // This function was created by James
{
    float oldlasty;
    float oldlastx;
    oldlastx = xrot;
    oldlasty = lasty;
    int diffx=x-lastx; //check the difference between the current x and the last x position
    int diffy=y-lasty; //check the difference between the current y and the last y position

    // This if else stops the camera from looking too far up or down
    if (lasty < 0)
				lasty = 0;
			if (lasty > 180)
				lasty = 180;
            else
            {
                xrot += (float) diffy; //set the xrot to xrot with the addition of the difference in the y position
            }

    lastx=x; //set lastx to the current x position
    lasty=y; //set lasty to the current y position
    yrot += (float) diffx;    //set the xrot to yrot with the addition of the difference in the x position
    if(yrot < -360)
        yrot += 360;
    if(yrot > 360)
        yrot -=360;
    if(xrot < -360)
        xrot += 360;
    if(xrot > 360)
        xrot -=360;
   // cout<<"xrot = "<<xrot<<"Yrot = "<<yrot<<"xpos = "<<xpos<<"ypos = "<<ypos<<endl;
    if (velmod == 3) // This part of code makes the ball follow the camnera along the y axis while held
    {
        // This part was made by Tim
        yrotradians = (float)((yrot-90)/180)*3.141592654; //degrees to radians, yrot-90 for the initial view point
        curpos[0] = xpos + 2*cos(yrotradians); //parametric equation
        curpos[2] = zpos + 2*sin(yrotradians); //parametric equation
        xrotradians = (float)((xrot-90)/180)*3.141592654; //degrees to radians, xrot-90 for the initial view point
        curpos[1] = ypos + -2*cos(xrotradians); //parametric equation
    }
}

int main (int argc, char **argv)
{
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize (1800, 1000);
    glutInitWindowPosition (0, 0);
    glutCreateWindow ("Project Projectile");
    readOFFFile("pin.off", &myPin);

    glutSetCursor(GLUT_CURSOR_NONE); // makes cursor invisable
    glutWarpPointer(900, 990); // sets cursor to ideal position on the screen
    initialPinLocation();
    glutDisplayFunc (display);
    glutTimerFunc(10, timer, 0);
    glutReshapeFunc (reshape);

    glutPassiveMotionFunc(mouseMovement); //check for mouse movement
    glutKeyboardFunc (keyboard);
    glutMainLoop ();
    return 0;
}
