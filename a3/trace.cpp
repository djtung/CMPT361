#include <stdio.h>
#include <GL/glut.h>
#include <math.h>
#include "global.h"
#include "sphere.h"
#include <iostream>
#include <sstream>

using namespace std;
//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];  

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int step_max;
extern int reflection_on;
extern int chessboard_on;
extern int supersample_on;
extern int stochastic_on;

//-----------------------------------------------------------------------------------------
float randfloat(float Min, float Max)
{
    return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

//-----------------------------------------------------------------------------------------
Spheres* initchessboard(){

  Point board_ctr = {5,0,0};
  float board_rad = 0;
  float board_ambient[] = {0.5, 0.5, 0.5};
  float board_diffuse[] = {0.3, 0.3, 0.3};
  float board_specular[] = {1.0, 1.0, 1.0};
  float board_shineness = 30;
  float board_reflectance = 1.5;
  Spheres* board = add_sphere(board, board_ctr, board_rad, board_ambient,
             board_diffuse, board_specular, board_shineness,
         board_reflectance, -1);
  return board;
}

//-----------------------------------------------------------------------------------------
//returns t value or -1.0 if no intersection
float intersect_board(Point o, Vector u, Point *hit) {
  //o.x, o.y, o.z = eye point
  //u.x, u.y, u.z = vector
  //*hit = intersection point
  const float board_width = 4;
  const float board_length = 4;
  const float bw_offset = 0;
  const float bl_offset = -7;

  Vector planenormal = {0,1,0};
  float A = planenormal.x;
  float B = planenormal.y;
  float C = planenormal.z;
  float D = 3;

  float t = -(((A*o.x)+(B*o.y)+(C*o.z)+D)/((A*u.x)+(B*u.y)+(C*u.z)));

  Point intersectionpoint = {0,0,0};

  intersectionpoint.x = o.x + t*u.x;
  intersectionpoint.y = o.y + t*u.y;
  intersectionpoint.z = o.z + t*u.z;

  if(intersectionpoint.x > -board_width+bw_offset && intersectionpoint.x < board_width+bw_offset && 
    intersectionpoint.z > -board_length+bl_offset && intersectionpoint.z < board_length+bl_offset &&
    t > 0)
  {
    hit->x = o.x + t*u.x;
    hit->y = o.y + t*u.y;
    hit->z = o.z + t*u.z;
    return t;
  }
  else
  {
    return -1.0;
  }

}

//-----------------------------------------------------------------------------------------
//generates random vector within 180 of surface normal
Vector randomvec(Vector surfnorm)
{
  Vector newvec = {0,0,0};

  do
  {
    newvec.x = randfloat(-10, 10);
    newvec.y = randfloat(-10, 10);
    newvec.z = randfloat(-10, 10);
  } while(vec_dot(newvec, surfnorm) <= 0);
  
  return newvec;
}

//-----------------------------------------------------------------------------------------
RGB_float add_diffspec(Point q, Vector v, Vector surf_norm, Spheres *sph)
{
  RGB_float color = {0,0,0};

  //light
  //light1 = position of light
  Vector light_vec = (get_vec(q, light1));
  normalize(&light_vec);
  float d = vec_len(light_vec);

  //vector R (reflected ray)
  float scaleamount = 2*(vec_dot(surf_norm, light_vec));
  Vector reflected_vec = vec_scale(surf_norm, scaleamount);
  reflected_vec = vec_minus(reflected_vec, light_vec);

  //decay term
  float decay = (1/((decay_a)+(decay_b*d)+(decay_c*pow(d,2))));

  //Id kd (n * l) (diffuse)
  float diffuser = light1_diffuse[0]*sph->mat_diffuse[0]*vec_dot(surf_norm, light_vec);
  float diffuseg = light1_diffuse[1]*sph->mat_diffuse[1]*vec_dot(surf_norm, light_vec);
  float diffuseb = light1_diffuse[2]*sph->mat_diffuse[2]*vec_dot(surf_norm, light_vec);

  //Is ks (r * v)^N (specular)
  float specularvalue = vec_dot(reflected_vec, v);
  if(specularvalue < 0)
  {
    specularvalue = 0;
  }

  float specularr = light1_specular[0]*sph->mat_specular[0]*pow(specularvalue, sph->mat_shineness);
  float specularg = light1_specular[1]*sph->mat_specular[1]*pow(specularvalue, sph->mat_shineness);
  float specularb = light1_specular[2]*sph->mat_specular[2]*pow(specularvalue, sph->mat_shineness);

  //phong 3rd term
  if(diffuser < 0) { diffuser = 0.f;}
  if(diffuseg < 0) { diffuseg = 0.f;}
  if(diffuseb < 0) { diffuseb = 0.f;}
  
  color.r += decay*(diffuser+specularr);
  color.g += decay*(diffuseg+specularg);
  color.b += decay*(diffuseb+specularb);
  
  return color;
}

//--------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres *sph) {
  //q = the point (to color)
  //v = eye vector
  //surf_norm = surface normal vector
  //sph = sphere the point is on

	RGB_float color = {0,0,0};

  //Iga kga (phong 1st term)
  color.r += global_ambient[0]*sph->reflectance;
  color.g += global_ambient[1]*sph->reflectance;
  color.b += global_ambient[2]*sph->reflectance;

  //Ia ka (phong 2nd term)
  color.r += light1_ambient[0]*sph->mat_ambient[0];
  color.g += light1_ambient[1]*sph->mat_ambient[1];
  color.b += light1_ambient[2]*sph->mat_ambient[2];

  RGB_float color2 = {0,0,0};
  Point rb;
  float buffer;

  //shadows. If shadows are not on, add the diffuse and specular terms to all pixels
  if(shadow_on == 1)
  {
    Vector feeler = get_vec(q, light1);
    if(intersect_scene(q, feeler, scene, &rb, sph, &buffer) == NULL)
    {
      color2 = add_diffspec(q, v, surf_norm, sph);
    }
  }
  else if(shadow_on != 1)
  {
    color2 = add_diffspec(q, v, surf_norm, sph);
  }

  color.r += color2.r;
  color.g += color2.g;
  color.b += color2.b;

	return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Point o, Vector ray, int level) {

  Point intersectionpoint;
  Point intersectionpoint2;
  float spheret;
  float boardt = -1.0;
  RGB_float color = {0,0,0};
  RGB_float recursreturn = {0,0,0};
  RGB_float phongreturn = {0,0,0};
  RGB_float stochasticreturn = {0,0,0};
  float scaleamount;
  Vector reflected_vec;
  Vector surf_norm;
  int i;

  Spheres* sph = intersect_scene(o, ray, scene, &intersectionpoint, NULL, &spheret);
  
  if(chessboard_on == 1)
  {
    Spheres* board = initchessboard();
    boardt = intersect_board(o, ray, &intersectionpoint2);
    //ray hits board only
    if(sph == NULL && boardt != -1.0)
    {
        sph = board;
        intersectionpoint = intersectionpoint2;
        if(int(intersectionpoint.x+4) % 2 == 0)
        {
          if(int(intersectionpoint.z+11) % 2 == 1)
          {
            color.r = 1;
            color.g = 1;
            color.b = 1;
          }
        }
        else
        {
          if(int(intersectionpoint.z+11) % 2 == 0)
          {
            color.r = 1;
            color.g = 1;
            color.b = 1;
          }
        }
    }
    //ray hits board and sphere, take closest
    else if(sph != NULL && boardt != -1.0)
    {
      //spheret is smaller (sphere is first intersection)
      if(spheret < boardt)
      {
        //do nothing
      }
      //boardt is smaller (board first intersection)
      else
      {
        sph = board;
        intersectionpoint = intersectionpoint2;
        if(int(intersectionpoint.x+4) % 2 == 0)
        {
          if(int(intersectionpoint.z+11) % 2 == 1)
          {
            color.r = 1;
            color.g = 1;
            color.b = 1;
          }
        }
        else
        {
          if(int(intersectionpoint.z+11) % 2 == 0)
          {
            color.r = 1;
            color.g = 1;
            color.b = 1;
          }
        }
      }
    }
    //ray hits sphere only
    else
    {
      //do nothing
    }
  }

  //color object
  if (sph != NULL) {

    //object surface normal
    if(sph->index > 0)
    {
      surf_norm = sphere_normal(intersectionpoint, sph);
    }
    else if (sph->index < 0)
    {
      surf_norm.x = 0;
      surf_norm.y = 1;
      surf_norm.z = 0;
    }
    
    //vector R (reflected ray)
    scaleamount = 2*(vec_dot(surf_norm, ray));
    reflected_vec = vec_scale(surf_norm, scaleamount);
    reflected_vec = vec_minus(ray, reflected_vec);

    //eye vector (from eye to point)
    Vector eye_vec = get_vec(intersectionpoint, eye_pos);
    normalize(&eye_vec);

    phongreturn = phong(intersectionpoint, eye_vec, surf_norm, sph);
    color.r += phongreturn.r;
    color.g += phongreturn.g;
    color.b += phongreturn.b;

    // recursive step for reflections
    if(sph->reflectance != 0 && level < step_max && reflection_on == 1)
    {
      intersectionpoint.x += 0.0001 * reflected_vec.x;
      intersectionpoint.y += 0.0001 * reflected_vec.y;
      intersectionpoint.z += 0.0001 * reflected_vec.z;

      recursreturn = recursive_ray_trace(intersectionpoint, reflected_vec, level+1);

      recursreturn.r *= sph->reflectance;
      recursreturn.g *= sph->reflectance;
      recursreturn.b *= sph->reflectance;

      color.r += recursreturn.r;
      color.g += recursreturn.g;
      color.b += recursreturn.b;
    }

    //stochastic step
    if(sph->reflectance != 0 && level < step_max && stochastic_on == 1)
    {
      if(reflection_on == 0)
      {
        intersectionpoint.x += 0.0001 * reflected_vec.x;
        intersectionpoint.y += 0.0001 * reflected_vec.y;
        intersectionpoint.z += 0.0001 * reflected_vec.z;
      }
      for(i=0; i<5; i++)
      {
        recursreturn = recursive_ray_trace(intersectionpoint, randomvec(surf_norm), level+1);

        recursreturn.r *= sph->reflectance;
        recursreturn.g *= sph->reflectance;
        recursreturn.b *= sph->reflectance;

        stochasticreturn.r += recursreturn.r;
        stochasticreturn.g += recursreturn.g;
        stochasticreturn.b += recursreturn.b;
      }
      
      stochasticreturn = clr_scale(stochasticreturn, (1/5.0));

      color.r += stochasticreturn.r;
      color.g += stochasticreturn.g;
      color.b += stochasticreturn.b;
    }

  }
  else if(sph == NULL && level == 0 && boardt == -1.0)
  {
    color = background_clr;
  }

	return color;

}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace() {
  int i, j, k;
  float x_grid_size = image_width / float(win_width);
  float y_grid_size = image_height / float(win_height);
  float x_start = -0.5 * image_width;
  float y_start = -0.5 * image_height;
  RGB_float ret_color;
  Point cur_pixel_pos;
  Vector ray;

  // ray is cast through center of pixel
  cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
  cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
  cur_pixel_pos.z = image_plane;

  //setup for super sampling
  RGB_float ss_sumcolor;
  Point ss_pixel_pos = {cur_pixel_pos.x,cur_pixel_pos.y,image_plane};

  for (i=0; i<win_height; i++) {
    for (j=0; j<win_width; j++) {
      ray = get_vec(eye_pos, cur_pixel_pos);

      //
      // You need to change this!!!

      // Parallel rays can be cast instead using below
      //
      // ray.x = ray.y = 0;
      // ray.z = -1.0;
      // ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

      // main color return
      ret_color = recursive_ray_trace(eye_pos, ray, 0);

      //supersampling if on
      if(supersample_on == 1)
      {
          ss_pixel_pos.x = cur_pixel_pos.x-x_grid_size*(1/4);
          ss_pixel_pos.y = cur_pixel_pos.y-y_grid_size*(1/4);
          ray = get_vec(eye_pos, ss_pixel_pos);
          ss_sumcolor = recursive_ray_trace(eye_pos, ray, 0);
          ret_color = clr_add(ret_color,ss_sumcolor);

          ss_pixel_pos.x = cur_pixel_pos.x-x_grid_size*(1/4);
          ss_pixel_pos.y = cur_pixel_pos.y+y_grid_size*(1/4);
          ray = get_vec(eye_pos, ss_pixel_pos);
          ss_sumcolor = recursive_ray_trace(eye_pos, ray, 0);
          ret_color = clr_add(ret_color,ss_sumcolor);

          ss_pixel_pos.x = cur_pixel_pos.x+x_grid_size*(1/4);
          ss_pixel_pos.y = cur_pixel_pos.y+y_grid_size*(1/4);
          ray = get_vec(eye_pos, ss_pixel_pos);
          ss_sumcolor = recursive_ray_trace(eye_pos, ray, 0);
          ret_color = clr_add(ret_color,ss_sumcolor);

          ss_pixel_pos.x = cur_pixel_pos.x+x_grid_size*(1/4);
          ss_pixel_pos.y = cur_pixel_pos.y-y_grid_size*(1/4);
          ray = get_vec(eye_pos, ss_pixel_pos);
          ss_sumcolor = recursive_ray_trace(eye_pos, ray, 0);
          ret_color = clr_add(ret_color,ss_sumcolor);

      }

      frame[i][j][0] = GLfloat(ret_color.r);
      frame[i][j][1] = GLfloat(ret_color.g);
      frame[i][j][2] = GLfloat(ret_color.b);

      cur_pixel_pos.x += x_grid_size;
    }

    cur_pixel_pos.y += y_grid_size;
    cur_pixel_pos.x = x_start;

  }

  
}
