#include "Grid.hpp"
#include <cfloat>

const double COLOR_MAX = 255;

void Grid::generateTerrain()
{
  PerlinNoise perlin;
  noiseMap = perlin.createMatrix3D(width, height, depth, 5);

  //perlin.smooth(noiseMap);

  for (double x = 0; x < width; x++)
  {
    for (double y = 0; y < height; y++)
    {
      for (double z = 0; z < height; z++)
      {
        double p = (*noiseMap)[x][y][z];
        double c = std::min(COLOR_MAX, p * 255);
        set(x, y, z, {c, c, c});
      }
    }
  }
}

void Grid::drawDensityMap(wxDC& dc, int w, int h, double lowerBound, double upperBound){
  auto densityMap = getDensityMap(lowerBound, upperBound);
  dc.SetPen(wxPen(wxColor(0, 0, 0), 1));

  int block_height = h / height;
  int block_width = w / width;

  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      double c = std::min(COLOR_MAX, (*densityMap)[x][y] * 255);
      dc.SetBrush(wxColor(c, c, c));

      dc.DrawRectangle(
        x * block_width, y * block_height, block_width - 1, block_height - 1);
    }
  }

}

//A material is considered any voxel between two values in the Perlin Noise map.
std::shared_ptr<matrix2d> Grid::getDensityMap(double lowerBound, double upperBound){
  std::shared_ptr<matrix2d> returnMatrix = std::make_shared<matrix2d>(width, std::vector<double>(height));
  //Get the count for each.
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      //Count the number of materials in the range.
      double count = 0;
      for (int z = 0; z < depth; z++)
      {
        double p = (*noiseMap)[x][y][z];
        if (p > lowerBound && p < upperBound)
        {
          count++;
        }
      }
      (*returnMatrix)[x][y] = count;
    }
  }

  //Get the min and max 
  double min = DBL_MAX;
  double max = -DBL_MAX;

  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      double p = (*returnMatrix)[x][y];
      min = (p < min) ? p : min;
      max = (p > max) ? p : max;
    }
  }

  //Using the min and max, equalize the return matrix to values between 0-1 based on density
  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      (*returnMatrix)[x][y] = ((*returnMatrix)[x][y] - min) / (max - min); 
    }
  }
  return returnMatrix;
}

void Grid::draw(wxDC& dc, int w, int h, int z)
{
  dc.SetPen(wxPen(wxColor(0, 0, 0), 1));

  int block_height = h / height;
  int block_width = w / width;

  for (int x = 0; x < width; x++)
  {
    for (int y = 0; y < height; y++)
    {
      Point p = at(x, y, z);
      dc.SetBrush(wxColor(p.x(), p.y(), p.z()));
      dc.DrawRectangle(
        x * block_width, y * block_height, block_width - 1, block_height - 1);
    }
  }
}
