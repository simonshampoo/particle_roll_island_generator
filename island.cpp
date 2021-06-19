#include <iostream>
#include <fstream>
#include <ctime>
#include "termcolor/termcolor.hpp"
#include <algorithm> //not sure if this is allowed
                     //but i only used it for the max() function to obtain max val in 2d arr
                     //if not allowed, then I could have just had simple "old max < current val" check
using namespace std;

//stage 0 -- intro
void printMenu();

//stage 1 -- setup
int **makeParticleMap(int width, int height, int numparticles, int maxLife);

void particleRoll(int **map, int width, int height, int i, int j, int maxLife);

int findMax(int **land, int width, int height);

void normalize(int **map, int width, int height, int maxVal);

//stage 2 -- construct island of chars
char **makeIsland(int width, int height);

void islandify(int **land, char **island, int height, int width, int waterLine, int landZone);

// stage 3 -- print, decorate, and write to file
void printCharIsland(char **land, int width, int height);

void printCharIslandToText(ofstream &o, int **land, char **island, int height, int width);

int width, height, xMax, xMin, yMax, yMin, numParticles, maxLife, waterLine;

int main()
{
    //I will be doing the extra credit with colored text
    //In the project directory, a folder 'termcolor' with the header file 'termcolor.hpp' must be in it
    srand(time(NULL));
    printMenu(); //print out the menu

    cout << "Enter the width of the island: ";
    cin >> width;
    if (width <= 0)
    {
        do
        {
            cout << "\nSorry, width cannot be <= 0. Please try again: ";
            cin >> width;
        } while (width <= 0); //input validation
    }

    cout << "\nEnter the height of the island: ";
    cin >> height;
    if (height <= 0)
    {
        do
        {
            cout << "\nSorry, height cannot be <= 0. Please try again: ";
            cin >> height;
        } while (height <= 0); //input validation
    }

    cout << "\nEnter minimum x: ";
    cin >> xMin;
    if (xMin < 0)
    {
        do
        {
            cout << "\nSorry, xMin cannot be negative. Please try again: ";
            cin >> xMin;
        } while (xMin < 0); //input validation
    }

    cout << "\nEnter maximum x: ";
    cin >> xMax;
    if (xMax < xMin)
    {
        do
        {
            cout << "\nSorry, xMax cannot be smaller than xMin. Pleae try again: ";
            cin >> xMax;
        } while (xMax < xMin); //input validation
    }
    else if (xMax > width)
    {
        do
        {
            cout << "\nSorry, xMax cannot be greater than the width. Please try again: ";
            cin >> xMax;
        } while (xMax > width); //input validation
    }

    cout << "\nEnter minimum y: ";
    cin >> yMin;
    if (yMin < 0)
    {
        do
        {
            cout << "\nSorry, yMin cannot be negative. Please try again: ";
            cin >> yMin;
        } while (yMin < 0); //input validation
    }

    cout << "\nEnter maximum y: ";
    cin >> yMax;
    if (yMax < yMin)
    {
        do
        {
            cout << "\nSorry, yMax cannot be smaller than yMin. Pleae try again: ";
            cin >> yMax;
        } while (yMax < yMin); //input validation
    }
    else if (yMax > height)
    {
        do
        {
            cout << "\nSorry, yMax cannot be greater than the height. Please try again: ";
            cin >> yMax;
        } while (yMax > height); //input validation
    }

    cout << "\nEnter number of particles to drop: ";
    cin >> numParticles;
    if (numParticles < 0)
    {
        do
        {
            cout << "\nSorry, you can't drop a negative amount of particles. Pleae try again: ";
            cin >> numParticles;
        } while (numParticles < 0); //input validation
    }

    cout << "\nEnter the maximum life of the particle: ";
    cin >> maxLife;
    if (maxLife < 0)
    {
        do
        {
            cout << "\nSorry, maxLife cannot be less than 0. Please try again: ";
            cin >> maxLife;
        } while (maxLife < 0); //input validation
    }

    cout << "\nEnter a value between 40 and 200 to use as the water line: ";
    cin >> waterLine;
    if (waterLine < 40 || waterLine > 200)
    {
        do
        {
            cout << "\nSorry, waterLine needs to be >= 40 or <= 200. Please re-enter: ";
            cin >> waterLine;
        } while (waterLine < 40 || waterLine > 200); //input validation
    }

    int **land = makeParticleMap(width, height, numParticles, maxLife); //create grid

    //bread and butter
    while (numParticles != 0) //while we have particles to drop
    {
        particleRoll(land, width, height, (xMin + xMax) / 2, (yMin + yMax) / 2, maxLife); //perform particle roll algorithm
        // start in the middle of our allowed xMin, xMax, yMin, yMax bounds
        numParticles--;
    }

    int maxN = findMax(land, width, height); //find max number in int 2d array

    normalize(land, width, height, maxN); //normalize our int 2d array with max number

    int landZone = 255 - waterLine; //create landZone so we know what to create given frequneices of land[i][j]

    char **island = makeIsland(width, height); //create char 2d array, skeleton of our island
    islandify(land, island, height, width, waterLine, landZone); //islandify our empty char 2d array 
    printCharIsland(island, width, height);  //print out the island to console looking all nice
    cout << termcolor::reset;     //reset colors

    ofstream myfile("island.txt");                              //now, lets just print it to a text file
    printCharIslandToText(myfile, land, island, height, width); //same principle as printCharIsland, but to a file
    myfile.close();

    return 0;
}

void printMenu()
{
    cout << "Welcome to the Island Generator!"
         << "\n"
         << "The generator is based on a particle roll algorithm where features are constructed based on how many times our particle travels over certain coordinates of our grid."
         << "\n"
         << "\nKey :"
         << "\n"
         << "\tDeep water = dark blue(#) (particle did not roll around these coords much, if at all)"
         << "\n"
         << "\tShallow water = bright blue(~)"
         << "\n"
         << "\tSand = yellow(.)"
         << "\n"
         << "\tGrass = green(-)"
         << "\n"
         << "\tForests = dark green(*)"
         << "\n"
         << "\tMountains = grey(^) (particle rolled around here the most)\n\n\n";
}

int **makeParticleMap(int width, int height, int numParticles, int maxLife)
{
    int **land;
    land = new int *[height];

    for (int i = 0; i < height; ++i)
    {
        land[i] = new int[width]; //we have the 2d array
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            land[i][j] = 0; //initialize, not sure if necessary
        }
    }

    return land;
}

void particleRoll(int **map, int width, int height, int x, int y, int maxLife) //dfs
{
    if (x < 0 || x >= height || y < 0 || y >= width || x < xMin || x > xMax || y < yMin || y > yMax || maxLife <= 0)
    {
        return; //invalid move or we are done with recursion
    }
    else
    {
        map[x][y]++; //rolled onto

        int xDir = (rand() % 3) - 1; // x direction will either be: -1, 0, +1
        int yDir = (rand() % 3) - 1; // y direction will either be: -1, 0, +1

        //recurse and roll around map w.r.t xDir and yDir, and decrement maxLife
        particleRoll(map, width, height, x + xDir, y + yDir, maxLife - 1);
    }
}

int findMax(int **land, int width, int height)
{
    int maxNum = -2147483647; //any number is bigger than this
    for (int x = 0; x < height; x++)
    {
        for (int y = 0; y < width; y++)
        {
            maxNum = max(land[x][y], maxNum); //first element will always be bigger than INT_MIN, then just keep checking afterwards
        }
    }
    return maxNum;
}

void normalize(int **map, int width, int height, int maxVal)
{
    for (int x = 0; x < height; x++)
    {
        for (int y = 0; y < width; y++)
        {
            float normalizedValue = ((float)map[x][y] / (float)maxVal) * 255; // if x,y int && y > x, x/y = 0, need to cast as float
            map[x][y] = (int)normalizedValue;                                 //cast back to int & normalize
        }
    }
}

char **makeIsland(int width, int height) //building our character 2d array
{
    char **island;
    island = new char *[height];

    for (int i = 0; i < height; i++)
    {
        island[i] = new char[width];
    }
    return island;
}

void islandify(int **land, char **island, int height, int width, int waterLine, int landZone)
{
    //the construction of our island based on particle rolling frequencies
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (land[i][j] < .5 * waterLine)
            {
                island[i][j] = '#';
            }
            else if (land[i][j] > (int)(.5 * waterLine) && land[i][j] <= waterLine)
            {
                island[i][j] = '~';
            }
            else
            {
                if (land[i][j] < waterLine + (int)(.15 * landZone))
                {
                    island[i][j] = '.';
                }
                else if (land[i][j] >= waterLine + (int)(.15 * landZone) && land[i][j] < (waterLine + (int)(.40 * landZone)))
                {
                    island[i][j] = '-';
                }
                else if (land[i][j] >= waterLine + (int)(.45 * landZone) && land[i][j] < (waterLine + (int)(.80 * landZone)))
                {
                    island[i][j] = '*';
                }
                else
                {
                    island[i][j] = '^';
                }
            }
        }
    }
}

void printCharIsland(char **land, int width, int height)
{
    //using termcolor, we print out to console with nice colors to clearly show how this represents an island
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (land[i][j] == '#')
            {
                cout << termcolor::on_blue << " " << termcolor::reset; //deep sea = dark blue
            }
            else if (land[i][j] == '~')
            {
                cout << termcolor::on_bright_blue << land[i][j] << termcolor::reset; //shallow sea = bright blue
            }
            else if (land[i][j] == '.')
            {
                cout << termcolor::on_yellow << land[i][j] << termcolor::reset; //sand = yellow
            }
            else if (land[i][j] == '-')
            {
                cout << termcolor::on_bright_green << land[i][j] << termcolor::reset; //grass = bright green
            }
            else if (land[i][j] == '*')
            {
                cout << termcolor::on_green << land[i][j] << termcolor::reset; //forest = green
            }
            else
            {
                cout << termcolor::on_bright_grey << land[i][j] << termcolor::reset; //mountains = grey
            }
        }
        cout << "\n";
    }
}

void printCharIslandToText(ofstream &o, int **land, char **island, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            o << island[i][j]; //write to file
        }
        o << "\n"; //newline
    }
}