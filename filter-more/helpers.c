#include <math.h>
#include "helpers.h"

void swap_RGBByte (BYTE *head, BYTE *tail);
void pixel_blur(int height, int width, int row, int column, RGBTRIPLE image[height][width]);
void pixel_edges(int height, int width, int row, int column, RGBTRIPLE image[height][width]);

// Convert image to grayscale
void grayscale(int height, int width, RGBTRIPLE image[height][width])
{
    // Parse pixels
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
        // Compute the avarage color rate
        int avg_color = round((float)(image[i][j].rgbtBlue + image[i][j].rgbtGreen + image[i][j].rgbtRed) / 3);
        //Set the color rate as new R;G;B
        image[i][j].rgbtRed = avg_color;
        image[i][j].rgbtBlue = avg_color;
        image[i][j].rgbtGreen = avg_color;
        }
    }
    return;
}

// Reflect image horizontally
void reflect(int height, int width, RGBTRIPLE image[height][width])
{
    // Parse pixels
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width - j - 1; j++)
        {
        // Swap colors
        swap_RGBByte(&image[i][j].rgbtRed, &image[i][width - j - 1].rgbtRed);
        swap_RGBByte(&image[i][j].rgbtBlue, &image[i][width - j - 1].rgbtBlue);
        swap_RGBByte(&image[i][j].rgbtGreen, &image[i][width - j - 1].rgbtGreen);
        }
    }
    return;
}

// Swap values between 2 color's BYTE from RGBTIPLE's variables
void swap_RGBByte (BYTE *head, BYTE *tail)
{
    BYTE tmp = *tail;
    *tail = *head;
    *head = tmp;
    return;
}

// Blur image
void blur(int height, int width, RGBTRIPLE image[height][width])
{
    //Call pixel_blur(int height, int width, int row, int column, RGBTRIPLE image[height][width]) targeting the 1st pixel
    pixel_blur(height, width, 0, 0, image);
    return;
}

// Blur a pixel based on his neighbor, using recursion before applying RGB
void pixel_blur(int height, int width, int row, int column, RGBTRIPLE image[height][width])
{
    // Base case: reached the last pixel at the bottom right
    if (row == height)
        return;
    int red_sum = 0;
    int green_sum = 0;
    int blue_sum = 0;
    int neighbor_count = 0;
    // Parse neighbor pixel from pixel[row-1][col-1] to pixel[row+1][col+1]
    for (int i = row - 1; i < row + 2; i++)
    {
        for (int j = column - 1; j < column + 2; j++)
        {
            // Check if neighbor pixel exist
            if (i > -1 && i < height && j > -1 && j < width)
            {
                // Increment the neighbor count for later average operation
                neighbor_count++;
                // Increment Red, Blue and Green sum
                red_sum += (int)(image[i][j].rgbtRed);
                green_sum += (int)(image[i][j].rgbtGreen);
                blue_sum += (int)(image[i][j].rgbtBlue);
            }
        }
    }
    // Recursive call to pixel_blur targeting next pixel, if j = width - 1 then j = 0
    pixel_blur(height, width, (row + ((column + 1) / width)), ((column + 1) % width),image);
    // Apply avg_Red, avg_Green, avg_Blue based on R/G/Bsum / neighbor_count
    image[row][column].rgbtRed = round((float)red_sum / neighbor_count);
    image[row][column].rgbtGreen = round((float)green_sum / neighbor_count);
    image[row][column].rgbtBlue = round((float)blue_sum / neighbor_count);
    return;
}

// Detect edges
void edges(int height, int width, RGBTRIPLE image[height][width])
{
    //Call pixel_edges(int height, int width, int row, int column, RGBTRIPLE image[height][width]) targeting the 1st pixel
    pixel_edges(height, width, 0, 0, image);
    return;
}

// Figures out if a pixel is at the edge of a subject hold in the image then highlight it accordingly
void pixel_edges(int height, int width, int row, int column, RGBTRIPLE image[height][width])
{
    // Base case: row == height - 1 && column == width - 1 then return
    if (row == height)
        return;
    // Initialize variables for later computing and assigning new RGB values
    int matrix_indicator = 0;
    int gxmatrix[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    int gymatrix[] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    // array[RED, GREEN, BLUE]
    int gxcolors[] = {0, 0, 0};
    int gycolors[] = {0, 0, 0};
    int computedcolors[] = {0, 0, 0};

    // Parse neighbor pixel from pixel[row-1][col-1] to pixel[row+1][col+1] to compute matrixes
    for (int i = row - 1; i < row + 2; i++)
    {
        for (int j = column - 1; j < column + 2; j++)
        {
            // Check if neighbor pixel exist, if pixel doesn't existe treat as full black pixel
            if (i > -1 && i < height && j > -1 && j < width)
            {
                // Increment Red, Blue and Green for GXcolors by image[i][j].R/G/B times the gxmatrix[matrix indicator]
                gxcolors[0] += (image[i][j].rgbtRed * gxmatrix[matrix_indicator]);
                gxcolors[1] += (image[i][j].rgbtGreen * gxmatrix[matrix_indicator]);
                gxcolors[2] += (image[i][j].rgbtBlue * gxmatrix[matrix_indicator]);
                // Increment Red, Blue and Green for GXcolors by image[i][j].R/G/B times the gymatrix[matrix indicator]
                gycolors[0] += (image[i][j].rgbtRed * gymatrix[matrix_indicator]);
                gycolors[1] += (image[i][j].rgbtGreen * gymatrix[matrix_indicator]);
                gycolors[2] += (image[i][j].rgbtBlue * gymatrix[matrix_indicator]);
            }
            // Increment matrix indicator
            matrix_indicator++;
        }
    }
    // Recursive call to pixel_edges targeting next pixel, if column = width - 1 then column = 0 and row++
    pixel_edges(height, width, (row + ((column + 1) / width)), ((column + 1) % width),image);
    // Apply and checks if the end result it over 255 to change values accordingly
    computedcolors[0] = round(sqrt((gxcolors[0] * gxcolors[0]) + (gycolors[0] * gycolors[0])));
    computedcolors[1] = round(sqrt((gxcolors[1] * gxcolors[1]) + (gycolors[1] * gycolors[1])));
    computedcolors[2] = round(sqrt((gxcolors[2] * gxcolors[2]) + (gycolors[2] * gycolors[2])));
    image[row][column].rgbtRed = (computedcolors[0] < 255 ? computedcolors[0] : 255);
    image[row][column].rgbtGreen = (computedcolors[1] < 255 ? computedcolors[1] : 255);
    image[row][column].rgbtBlue = (computedcolors[2] < 255 ? computedcolors[2] : 255);
    return;
}