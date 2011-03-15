#include "Player.h"
#include "UserInput.h"

Player::Player(unsigned playerID,
               Vector initialPosition) : mPlayerID(playerID)
                                       , activeInputs(0)
                                       , winLossState(0)
                                       , scale(1.0)
{
}

void
Player::setTransform(const btTransform &trans)
{
    //Matrix transform;
    transform.LoadIdentity();
    Vector origin(trans.getOrigin());
    transform.Translate(origin.x, origin.y, origin.z);
    btMatrix3x3 rotation(trans.getRotation());
    Matrix ourRotation(rotation);
    transform = transform.MMProduct(ourRotation);
    //mPlayerNode->SetTransform(transform);
}

void
Player::applyInput(UserInput& input)
{
    // Separate our inputs into those that begin an action and those
    // that end an action.
    uint32_t begins = input.inputs & 0x55555555;
    uint32_t ends = input.inputs & 0xAAAAAAAA;

    // Sanity check - We shouldn't begin and end the same input
    // NOTE - apparently SFML does this sometimes. Oh well!
    //assert(((ends >> 1) & begins) == 0);

    // Sanity check - We shouldn't begin anything already begun
    // NOTE - this is disabled because operating systems actually send multiple
    // KeyPressed events after a certain delay when holding down a key.
    // assert((begins & activeInputs) == 0);

    // Sanity check - We shouldn't end anything not begun
    // NOTE - this sometimes screws up too, probably when SFML misses
    // the keydown. Ah well...
    // assert(((ends >> 1) | activeInputs) == activeInputs);

    // Apply falcon inputs
    activeFalconInputs = input.falconInputs;
    // Apply our begins
    activeInputs |= begins;

    // Apply our ends
    activeInputs &= ~(ends >> 1);
}

void
Player::Render() {
    
    //GL_CHECK(glActiveTexture(GL_TEXTURE0));
    //GL_CHECK(glDisable(GL_TEXTURE_2D));
    //glEnable(GL_LIGHTING);
    //glEnable(GL_BLEND);
    
    //glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    //glLoadIdentity();
    
    glColor4f(1.0, 0.0, 0.0, 1.0);
    //float color1[] = {0.866, 0.113, 0.090};
    //float color2[] = {0.788, 0.913, 0.086};
    
    //move the ball to the current position
    //glTranslatef(curPos[0],curPos[1],curPos[2]);
    //glTranslatef(0,5,0);
    
    GLfloat mat[16];
    transform.Get(mat);
    glMultMatrixf(mat);
    
    //rotate the ball around its center
    //glRotatef(rotX, 1, 0, 0);
    //glRotatef(-rotZ, 0, 0, 1);
    
    //draw a sphere
    GLUquadric* sphere = gluNewQuadric();
    gluQuadricDrawStyle(sphere, GLU_LINE);
    gluQuadricNormals(sphere, GLU_SMOOTH);
    
    // set the color of the ball
    glColor4f(1.0, 0.0, 0.0, 1.0);
    /* If we ever changed/used the texture or orientation state
     of quadObj, we'd need to change it to the defaults here
     with gluQuadricTexture and/or gluQuadricOrientation. */
    gluSphere(sphere, scale, 16, 16);
    gluDeleteQuadric(sphere);
    
    glPopMatrix();
}
