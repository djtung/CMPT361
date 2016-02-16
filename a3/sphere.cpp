#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <sstream>

using namespace std;

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection, 
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
float intersect_sphere(Point o, Vector u, Spheres *sph, Point *hit) {
  //o.x, o.y, o.z = eye point
  //u.x, u.y, u.z = vector
  //sph->center.x, sph->center.y, sph->center.z = sphere center coord
  float A = (u.x*u.x)+(u.y*u.y)+(u.z*u.z);
  float B = (2*u.x*(o.x-sph->center.x))+(2*u.y*(o.y-sph->center.y))+(2*u.z*(o.z-sph->center.z));
  float C = pow((o.x-sph->center.x), 2)+pow((o.y-sph->center.y), 2)+ 
            pow((o.z-sph->center.z), 2)-pow((sph->radius), 2);

  float disc = (B*B)-(4*A*C);
  float t1, t2;

  if(disc>=0)
  {
    t1 = ((-B)+sqrt(disc))/(2*A);
    t2 = ((-B)-sqrt(disc))/(2*A); 
  }
  else
  {
    return -1.0;
  }

  float thit;

  if(t1 < t2 && t1 >= 0)
  {
    thit = t1;
    hit->x = o.x + thit*u.x;
    hit->y = o.y + thit*u.y;
    hit->z = o.z + thit*u.z;
    return thit;
  }
  else if(t2 <= t1 && t2 >= 0)
  {
    thit = t2;
    hit->x = o.x + thit*u.x;
    hit->y = o.y + thit*u.y;
    hit->z = o.z + thit*u.z;
    return thit;
  }

  return -1.0;

}
/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For example, note that you
 * should return the point of intersection to the calling function.

  set ignorefirst = a sphere you want to ignore (for first intersection), NULL if none
 **********************************************************************/
Spheres *intersect_scene(Point o, Vector u, Spheres *sph, Point *hit, Spheres *ignorefirst, float* t) {
  float detect;
  float mint = 1000.0; //arbitrary large number
  Spheres *firsthit = NULL;

  while(sph != NULL)
  {
      detect = intersect_sphere(o, u, sph, hit);

      if(detect < mint && detect != -1.0)
      {
        if(ignorefirst == NULL)
        {
          mint = detect;
          firsthit = sph;
          *t = mint;
        }
        else
        {
          if(sph->index != ignorefirst->index)
          {
            mint = detect;
            firsthit = sph;
            *t = mint;
          }
        }
      }
      sph = sph->next;
  }

  return firsthit; 
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres *add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
		    float dif[], float spe[], float shine, 
		    float refl, int sindex) {
  Spheres *new_sphere;

  new_sphere = (Spheres *)malloc(sizeof(Spheres));
  new_sphere->index = sindex;
  new_sphere->center = ctr;
  new_sphere->radius = rad;
  (new_sphere->mat_ambient)[0] = amb[0];
  (new_sphere->mat_ambient)[1] = amb[1];
  (new_sphere->mat_ambient)[2] = amb[2];
  (new_sphere->mat_diffuse)[0] = dif[0];
  (new_sphere->mat_diffuse)[1] = dif[1];
  (new_sphere->mat_diffuse)[2] = dif[2];
  (new_sphere->mat_specular)[0] = spe[0];
  (new_sphere->mat_specular)[1] = spe[1];
  (new_sphere->mat_specular)[2] = spe[2];
  new_sphere->mat_shineness = shine;
  new_sphere->reflectance = refl;
  new_sphere->next = NULL;

  if (slist == NULL) { // first object
    slist = new_sphere;
  } else { // insert at the beginning
    new_sphere->next = slist;
    slist = new_sphere;
  }

  return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres *sph) {
  Vector rc;

  rc = get_vec(sph->center, q);
  normalize(&rc);
  return rc;
}
