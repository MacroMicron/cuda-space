#include "Bool.h"
#include "UserSettings.h"
#include "MathLinearStructures.cuh"
#include "MathLinear.cuh"
#include <malloc.h>
#include <assert.h>
#include <math.h>

#ifdef _WIN32
#include <Windows.h>
#endif

//temporary for printf
#include <stdio.h>

#include "device_launch_parameters.h"
#ifndef _CUDA_RUNTIME_
#define _CUDA_RUNTIME_
#include "cuda_runtime.h"
#endif

#include "math.h"

//#define PRECISION 0.00001
#define PRECISION 0.1


#define SGN(a)		( (a)>0 ? 1 : ((a)<0?-1:0) )

#define Z_BETWEEN_X_AND_Y(z,x,y)	((x)>(y) ? ((z)>=(y))&&((z)<=(x)) : ((z)<=(y))&&((z)>=(x)))
//bugfix
#define X_NEAR_Y(x,y)	Z_BETWEEN_X_AND_Y((x),(y)-PRECISION,(y)+PRECISION)

#define NEAR_0(x)	X_NEAR_Y((x),0)

#define POINT_IN_PLANE(point, plane)	\
			(NEAR_0(  (  (plane)[0]*(point)[0] + (plane)[1]*(point)[1] + (plane)[2]*(point)[2] + (plane)[3]  )  ))

//is a helper-macros for ON_ONE_SIDE
#define LINE(x, y, p1x_line, p1y_line, p2x_line, p2y_line)	\
			(   (x)*((p1y_line)-(p2y_line))  -  (y)*((p1x_line)-(p2x_line))  +  (p1x_line)*(p2y_line)-(p2x_line)*(p1y_line)  )

//function for four points to define one what side is situaded first and second point relarive to line (third and fourth points)
#define ON_ONE_SIDE(p1x, p1y, p2x, p2y, p1x_line, p1y_line, p2x_line, p2y_line)		\
			((  SGN( LINE(p1x, p1y, p1x_line, p1y_line, p2x_line, p2y_line) )*			\
			    SGN( LINE(p2x, p2y, p1x_line, p1y_line, p2x_line, p2y_line) )   )>=0)

//define plane for projection, coordinate's - out parametrs
#define CHOISE_PROJECTION(polygon_plane, coordinate1, coordinate2)	\
			if ((polygon_plane)[2])  {  (coordinate1)=0;  (coordinate2)=1;  }					\
			else if((polygon_plane)[1]) { (coordinate1)=0;  (coordinate2)=2; }				\
			else {  (coordinate1)=1;  (coordinate2)=2;  }

//is point in segment? for 3D ok; points - are pointers
#define POINT_IN_SEGMENT_3D(point, seg_point1, seg_point2)	\
			(  ((seg_point1)[0]!=(seg_point2)[0])  ?  ( Z_BETWEEN_X_AND_Y((point)[0],(seg_point1)[0],(seg_point2)[0]) )				\
			:  ((seg_point1)[1]!=(seg_point2)[1]   ?  ( Z_BETWEEN_X_AND_Y((point)[1],(seg_point1)[1],(seg_point2)[1]) )				\
												   :  ( Z_BETWEEN_X_AND_Y((point)[2],(seg_point1)[2],(seg_point2)[2]) ) ) )

#define POINT_IN_SEGMENT_2D(px,py, sp1x,sp1y, sp2x,sp2y)	\
			( ((sp1x)!=(sp2x))  ?  ( Z_BETWEEN_X_AND_Y((px),(sp1x),(sp2x)) )					\
								:  ( Z_BETWEEN_X_AND_Y((py),(sp1y),(sp2y)) ))

//is segments are intersected (segments on one line)? for 1D and 2D and 3D ok
#define SEGMENTS_INTERSECT_ON_LINE_3D(seg1p1, seg1p2, seg2p1, seg2p2)	\
			(  POINT_IN_SEGMENT_3D((seg1p1),(seg2p1),(seg2p2))									\
			|| POINT_IN_SEGMENT_3D((seg1p2),(seg2p1),(seg2p2))									\
			|| POINT_IN_SEGMENT_3D((seg2p1),(seg1p1),(seg1p2))									\
			|| POINT_IN_SEGMENT_3D((seg2p2),(seg1p1),(seg1p2)) )

//define line coefficients abc on the plane through two points: p1 and p2; a,b,c - out parametrs must be declared before
#define LINE_DEFINE(a, b, c, p1x, p1y, p2x, p2y)	{  a=p1y-p2y;   b=p2x-p1x;   c=-a*p1x-b*p1y;  }

//divide segment into parts in the ratio (lyambda):(1-lyambda) from the point1 to point2
//return x0, y0, z0
#define SEGMENT_DIVIDE(x0, y0, z0,  lyambda,  x1, y1, z1,   x2, y2, z2)							\
								{ (x0) = (x1) + (lyambda)*((x2)-(x1));									\
								  (y0) = (y1) + (lyambda)*((y2)-(y1));									\
								  (z0) = (z1) + (lyambda)*((z2)-(z1));}

//get the centroid coordinates in the triangle p1, p2, p3
//p1, p2, p3 should situated in one plane!
//return out: x0, y0, z0
#define GET_CENTROID(x0,y0,z0,  x1,y1,z1,  x2,y2,z2,  x3,y3,z3)			SEGMENT_DIVIDE((x0),(y0),(z0),  2.0/3.0,    (x1), (y1), (z1),   \
																				((x2)+(x3))/2.0, ((y2)+(y3))/2.0, ((z2)+(z3))/2.0   )

//get the centroid coordinates in the quadropolygon p1, p2, p3, p4
//p1, p2, p3, p4 should be in one plane!
//quadropolygon should be convex!
//return out: x0, y0, z0
#define GET_CENTROID_4(x0,y0,z0, x1,y1,z1, x2,y2,z2, x3,y3,z3, x4,y4,z4)								\
								{ (x0) = ((x1)+(x2)+(x3)+(x4))/4.0;					\
								  (y0) = ((y1)+(y2)+(y3)+(y4))/4.0;					\
								  (z0) = ((z1)+(z2)+(z3)+(z4))/4.0;}


//if we have segment [(1,1,1);(2,2,2)] => [(1.0003,1.0003,1.0003);(2,2,2)] whereif eps = 0.0001 (show PRECISION); on 3*eps value change
//change only first point (!) => input: x1,y1,z1 ,x2,y2,z2; output: x1,y1,z1 (just change x1,y1,z1)
#define DECREASE_SEGMENT_TO_INTERVAL_3D(x1,y1,z1,  x2,y2,z2)									\
								{ (x1) = (x1) + (3.0*(PRECISION)*( (x2) - (x1) )/ ( ((x2)-(x1))*((x2)-(x1)) + ((y2)-(y1))*((y2)-(y1)) + ((z2)-(z1))*((z2)-(z1)) ));			\
								  (y1) = (y1) + (3.0*(PRECISION)*( (y2) - (y1) )/ ( ((x2)-(x1))*((x2)-(x1)) + ((y2)-(y1))*((y2)-(y1)) + ((z2)-(z1))*((z2)-(z1)) ));			\
								  (z1) = (z1) + (3.0*(PRECISION)*( (z2) - (z1) )/ ( ((x2)-(x1))*((x2)-(x1)) + ((y2)-(y1))*((y2)-(y1)) + ((z2)-(z1))*((z2)-(z1)) ));}

//interval ro between A and B
#define INTERVAL_BETWEEN_A_AND_B(ax,ay,az, bx,by,bz)										\
								 sqrt(   ((ax)-(bx))*((ax)-(bx)) +  ((ay)-(by))*((ay)-(by)) +  ((az)-(bz))*((az)-(bz))  )


//return true if segments intersected
__host__ __device__
bool IsSegmentsIntersected2D(real seg1p1x, real seg1p1y, real seg1p2x, real seg1p2y,
							 real seg2p1x, real seg2p1y, real seg2p2x, real seg2p2y)
{
	real a1,b1,c1, a2,b2,c2, kramer_determinant, IntersectionPoint[2];
	LINE_DEFINE(a1,b1,c1, seg1p1x,seg1p1y, seg1p2x,seg1p2y);
	LINE_DEFINE(a2,b2,c2, seg2p1x,seg2p1y, seg2p2x,seg2p2y);
	kramer_determinant=a1*b2-b1*a2;
	if (kramer_determinant) { //not parallel and not union
		IntersectionPoint[0]=(b1*c2-c1*b2)/kramer_determinant;
		IntersectionPoint[1]=(c1*a2-a1*c2)/kramer_determinant;
		if (POINT_IN_SEGMENT_2D(IntersectionPoint[0],IntersectionPoint[1], seg1p1x,seg1p1y, seg1p2x, seg1p2y))
			if (POINT_IN_SEGMENT_2D(IntersectionPoint[0],IntersectionPoint[1], seg2p1x,seg2p1y, seg2p2x, seg2p2y))
				return true;
	}
	else if (!(a1!=0 ? a1*c2-c1*a2: b1*c2-c1*b2)) { //equal lines, but what about segments?
		if (POINT_IN_SEGMENT_2D(seg1p1x,seg1p1y,seg2p1x,seg2p1y,seg2p2x,seg2p2y)||
			POINT_IN_SEGMENT_2D(seg1p2x,seg1p2y,seg2p1x,seg2p1y,seg2p2x,seg2p2y)||
			POINT_IN_SEGMENT_2D(seg2p1x,seg2p1y,seg1p1x,seg1p1y,seg1p2x,seg1p2y)||
			POINT_IN_SEGMENT_2D(seg2p2x,seg2p2y,seg1p1x,seg1p1y,seg1p2x,seg1p2y))
			return true;
	}
	return false;
}

//founding plane by array of points
//plane - out parametr; size(plane[])=4
__host__ __device__
void PlaneDefine(CalcFace *face)
{
	___PlaneFrom(face)[0] = ___YPointFrom(face, 1) * ( ___ZPointFrom(face, 2) - ___ZPointFrom(face, 0) ) +
						    ___YPointFrom(face, 0) * ( ___ZPointFrom(face, 1) - ___ZPointFrom(face, 2) ) +
					   	    ___YPointFrom(face, 2) * ( ___ZPointFrom(face, 0) - ___ZPointFrom(face, 1) );

	___PlaneFrom(face)[1] = - (
							  ___XPointFrom(face, 1) * ( ___ZPointFrom(face, 2) - ___ZPointFrom(face, 0) ) +
							  ___XPointFrom(face, 0) * ( ___ZPointFrom(face, 1) - ___ZPointFrom(face, 2) ) +
							  ___XPointFrom(face, 2) * ( ___ZPointFrom(face, 0) - ___ZPointFrom(face, 1) ) );

	___PlaneFrom(face)[2] =   ___XPointFrom(face, 1) * ( ___YPointFrom(face, 2) - ___YPointFrom(face, 0) ) +
							  ___XPointFrom(face, 0) * ( ___YPointFrom(face, 1) - ___YPointFrom(face, 2) ) +
							  ___XPointFrom(face, 2) * ( ___YPointFrom(face, 0) - ___YPointFrom(face, 1) );

	___PlaneFrom(face)[3] = - ___XPointFrom(face, 0) * ___PlaneFrom(face)[0]
							- ___YPointFrom(face, 0) * ___PlaneFrom(face)[1]
							- ___ZPointFrom(face, 0) * ___PlaneFrom(face)[2] ;
}

//only for convex polygons!
//need non-convex -to-> convex use operator before
__host__ __device__
void DefineConvexSquare(CalcFace *face)
{
	real a, b, c, p;
	integer i;
	face->Square = 0.0;
	for (i=2; i < ___DimOf(face); i++)
	{
		//a = ro between 0 and i-1
		a = INTERVAL_BETWEEN_A_AND_B(___XPointFrom(face, 0), ___YPointFrom(face, 0), ___ZPointFrom(face, 0), 		___XPointFrom(face, i-1), ___YPointFrom(face, i-1), ___ZPointFrom(face, i-1));
		//b = ro between 0 and i-1
		b = INTERVAL_BETWEEN_A_AND_B(___XPointFrom(face, 0), ___YPointFrom(face, 0), ___ZPointFrom(face, 0),            ___XPointFrom(face, i), ___YPointFrom(face, i), ___ZPointFrom(face, i));
		//c = ro between i and i-1
		c = INTERVAL_BETWEEN_A_AND_B(___XPointFrom(face, i), ___YPointFrom(face, i), ___ZPointFrom(face, i),            ___XPointFrom(face, i-1), ___YPointFrom(face, i-1), ___ZPointFrom(face, i-1));
		p = (a+b+c) /2;
		face->Square += sqrt(p*(p-a)*(p-b)*(p-c));
	}
}

//true if Quadrilateral without intersection
//only for 4-points Quadrilateral
//TODO:need for all non-convex polyogns: non-convex -to-> convex.
__host__ __device__
void CorrectQuadrilateralSimplisity(CalcFace *face)
{
	//we need in proection:
	unsigned char coordinate1, coordinate2;
	bool flag;
	CHOISE_PROJECTION(___PlaneFrom(face), coordinate1, coordinate2);
	if (IsSegmentsIntersected2D(___XYZPointFrom(face, 0, coordinate1), ___XYZPointFrom(face, 0, coordinate2),
								___XYZPointFrom(face, 1, coordinate1), ___XYZPointFrom(face, 1, coordinate2),
								___XYZPointFrom(face, 2, coordinate1), ___XYZPointFrom(face, 2, coordinate2),
								___XYZPointFrom(face, 3, coordinate1), ___XYZPointFrom(face, 3, coordinate2)))
			flag=false;
	else if (IsSegmentsIntersected2D(___XYZPointFrom(face, 0, coordinate1), ___XYZPointFrom(face, 0, coordinate2),
									 ___XYZPointFrom(face, 3, coordinate1), ___XYZPointFrom(face, 3, coordinate2),
									 ___XYZPointFrom(face, 1, coordinate1), ___XYZPointFrom(face, 1, coordinate2),
									 ___XYZPointFrom(face, 2, coordinate1), ___XYZPointFrom(face, 2, coordinate2)))
			flag=true;
	else return;
	___ExchangeVertices(flag?2:0, 3, face);
}

//size >=3 - is dimension (for 3-polygon or 4-polygon for example)
//you can use it only for 3-dim or 4-dim or any-dim convex
__host__ __device__
Bool PointInPolygon(CalcVertex *point, CalcFace *face)
{
	if (POINT_IN_PLANE((real*)point, ___PlaneFrom(face)))		//\EF\EE\E4 \E8\ED\F2\E5\F0\F4\E5\E9\F1
	{
		//checking point of intersection in polygon (by projection)
		//choose the plane for projection:
		unsigned char coordinate1, coordinate2;
		CHOISE_PROJECTION(___PlaneFrom(face), coordinate1, coordinate2);
		if (___DimOf(face)==4) { 
			//if non-convex 4-dimension
			//need in manual control do different convex and non-convex => copy of function IsSegmentIntersected2D:
			real a1,b1,c1, a2,b2,c2, kramer_determinant, IntersectionPoint[2], triag[2][9]; //\EF\F0\E5\EE\E1\F0\E0\E7\EE\E2\E0\F2\FC \E2 \F2\EE\F7\EA\E8
			bool diag[2];
			LINE_DEFINE(a1,b1,c1,
				___XYZPointFrom(face, 0, coordinate1), ___XYZPointFrom(face, 0, coordinate2),
				___XYZPointFrom(face, 2, coordinate1), ___XYZPointFrom(face, 2, coordinate2));
			LINE_DEFINE(a2,b2,c2,
				___XYZPointFrom(face, 1, coordinate1), ___XYZPointFrom(face, 1, coordinate2),
				___XYZPointFrom(face, 3, coordinate1), ___XYZPointFrom(face, 3, coordinate2));
			kramer_determinant=a1*b2-b1*a2; //diagonal lines intersected in all ways of life
			IntersectionPoint[0]=(b1*c2-c1*b2)/kramer_determinant;
			IntersectionPoint[1]=(c1*a2-a1*c2)/kramer_determinant;
			diag[0]=POINT_IN_SEGMENT_2D(IntersectionPoint[0],IntersectionPoint[1],
				___XYZPointFrom(face, 0, coordinate1), ___XYZPointFrom(face, 0, coordinate2),
				___XYZPointFrom(face, 2, coordinate1), ___XYZPointFrom(face, 2, coordinate2));
			diag[1]=POINT_IN_SEGMENT_2D(IntersectionPoint[0],IntersectionPoint[1],
				___XYZPointFrom(face, 1, coordinate1), ___XYZPointFrom(face, 1, coordinate2),
				___XYZPointFrom(face, 3, coordinate1), ___XYZPointFrom(face, 3, coordinate2));
			//condition of non-convex 4-dimension polygon:
			if (diag[0] != diag[1]) {
				if (!diag[0]) { //it's ok that coordinate3 not used because in recurse function other CHOISE_PROJECTION and coordinate3 never used
					triag[1][0*3+coordinate1] = triag[0][0*3+coordinate1] = ___XYZPointFrom(face, 0, coordinate1);
					triag[1][0*3+coordinate2] = triag[0][0*3+coordinate2] = ___XYZPointFrom(face, 0, coordinate2);
					triag[1][1*3+coordinate1] = triag[0][1*3+coordinate1] = ___XYZPointFrom(face, 2, coordinate1);
					triag[1][1*3+coordinate2] = triag[0][1*3+coordinate2] = ___XYZPointFrom(face, 2, coordinate2);
					triag[0][2*3+coordinate1]= ___XYZPointFrom(face, 1, coordinate1);
					triag[0][2*3+coordinate2]= ___XYZPointFrom(face, 1, coordinate2);
					triag[1][2*3+coordinate1]= ___XYZPointFrom(face, 3, coordinate1);
					triag[1][2*3+coordinate2]= ___XYZPointFrom(face, 3, coordinate2);
				}
				else {
					triag[1][0*3+coordinate1] = triag[0][0*3+coordinate1] = ___XYZPointFrom(face, 1, coordinate1);
					triag[1][0*3+coordinate2] = triag[0][0*3+coordinate2] = ___XYZPointFrom(face, 1, coordinate2);
					triag[1][1*3+coordinate1] = triag[0][1*3+coordinate1] = ___XYZPointFrom(face, 3, coordinate1);
					triag[1][1*3+coordinate2] = triag[0][1*3+coordinate2] = ___XYZPointFrom(face, 3, coordinate2);
					triag[0][2*3+coordinate1]=___XYZPointFrom(face, 2, coordinate1);
					triag[0][2*3+coordinate2]=___XYZPointFrom(face, 2, coordinate2);
					triag[1][2*3+coordinate1]=___XYZPointFrom(face, 0, coordinate1);
					triag[1][2*3+coordinate2]=___XYZPointFrom(face, 0, coordinate2);
				}
				unsigned char k;
				for(unsigned char j=0; j<2; j++) {
					k=0; diag[j]=true;
					while (diag[j]&&(k<3)) {
						if ( !ON_ONE_SIDE(
								___XYZOf(point, coordinate1), ___XYZOf(point, coordinate2),
								triag[j][k*3+coordinate1],triag[j][k*3+coordinate2],
								triag[j][(k+1)%3*3+coordinate1],triag[j][(k+1)%3*3+coordinate2],
								triag[j][(k+2)%3*3+coordinate1],triag[j][(k+2)%3*3+coordinate2]))
							diag[j]=false; //flag==true if point in triangle and false if no
						k++;
					}
				}
				return (diag[0]||diag[1]);	//no recusre in CUDA - no "return (PointInPolygon(point, triag1, polygon_plane, 3)||PointInPolygon(point, triag2, polygon_plane, 3)); //trianglyed it!"
			}
		}
		//DEBUG
		//coordinate1 = 1;
		//coordinate2 = 2;
		for (unsigned char i=0; i < ___DimOf(face); i++) {			
			if ( !ON_ONE_SIDE(
					___XYZOf(point, coordinate1), ___XYZOf(point, coordinate2),
					___XYZPointFrom(face, i, coordinate1),
					___XYZPointFrom(face, i, coordinate2),
					___XYZPointFrom(face, (i+1) % ___DimOf(face), coordinate1),
					___XYZPointFrom(face, (i+1) % ___DimOf(face), coordinate2),
					___XYZPointFrom(face, (i+2) % ___DimOf(face), coordinate1),
					___XYZPointFrom(face, (i+2) % ___DimOf(face), coordinate2))
			   ) return false;
//DEBUG			
			printf("%d for (%f,%f) and (%f,%f)\n", 			
			!ON_ONE_SIDE(
                                        ___XYZOf(point, coordinate1), ___XYZOf(point, coordinate2),
                                        ___XYZPointFrom(face, 0, coordinate1),
                                        ___XYZPointFrom(face, 0, coordinate2),
                                        ___XYZPointFrom(face, (0+1) % ___DimOf(face), coordinate1),
                                        ___XYZPointFrom(face, (0+1) % ___DimOf(face), coordinate2),
                                        ___XYZPointFrom(face, (0+2) % ___DimOf(face), coordinate1),
                                        ___XYZPointFrom(face, (0+2) % ___DimOf(face), coordinate2)),
			___XYZPointFrom(face, 0, coordinate1),  ___XYZPointFrom(face, 0, coordinate2),
			___XYZOf(point, coordinate1), ___XYZOf(point, coordinate2)			
			);
			

		}
		return true;
	}
	return false;
}

//if (segment[point1, point2] interset Polygon)
__host__ __device__
Bool IsSegmentIntersectPolygon(CalcVertex *point1, CalcVertex *point2,
							   CalcFace *face)
{
	real  temp = - ( ___PlaneFrom(face)[0] * ( ___XOf(point2) - ___XOf(point1) ) +
					 ___PlaneFrom(face)[1] * ( ___YOf(point2) - ___YOf(point1) ) +
					 ___PlaneFrom(face)[2] * ( ___ZOf(point2) - ___ZOf(point1) ) );
	CalcVertex IntersectionPoint;
	if (!NEAR_0(temp)) //if polygon and segment are not parallel
	{
			IntersectionPoint.x=(  ___PlaneFrom(face)[3] * (___XOf(point2) - ___XOf(point1)) +
								   ___PlaneFrom(face)[1] * (___YOf(point1)*___XOf(point2) - ___XOf(point1)*___YOf(point2)) +
								   ___PlaneFrom(face)[2] * (___ZOf(point1)*___XOf(point2) - ___XOf(point1)*___ZOf(point2)) ) / temp;
			IntersectionPoint.y=(  ___PlaneFrom(face)[3] * (___YOf(point2) - ___YOf(point1)) +
				                   ___PlaneFrom(face)[0] * (___XOf(point1)*___YOf(point2) - ___YOf(point1)*___XOf(point2)) +
								   ___PlaneFrom(face)[2] * (___ZOf(point1)*___YOf(point2) - ___YOf(point1)*___ZOf(point2)) ) / temp;
			IntersectionPoint.z=(  ___PlaneFrom(face)[3] * (___ZOf(point2) - ___ZOf(point1)) +
				                   ___PlaneFrom(face)[0] * (___XOf(point1)*___ZOf(point2) - ___ZOf(point1)*___XOf(point2)) +
								   ___PlaneFrom(face)[1] * (___YOf(point1)*___ZOf(point2) - ___ZOf(point1)*___YOf(point2)) ) / temp;
			//if IntersectionPoint in segment:
			if (POINT_IN_SEGMENT_3D( (real*)&IntersectionPoint, (real*)point1, (real*)point2 ))
				if (PointInPolygon(&IntersectionPoint, face)) 
							return true;
	}
	else {
		if ((point1->x == point2->x)&&(point1->y == point2->y)&&(point1->z == point2->z))
			return PointInPolygon(point1, face);
		else if (POINT_IN_PLANE((real*)&point1, ___PlaneFrom(face)))
		{//we need in proection:
			unsigned char coordinate1, coordinate2;
			CHOISE_PROJECTION(___PlaneFrom(face), coordinate1, coordinate2);
			for (unsigned char i=0; i < ___DimOf(face); i++)
				if (IsSegmentsIntersected2D( ___XYZOf(point1, coordinate1), ___XYZOf(point1, coordinate2),
						___XYZOf(point2, coordinate1), ___XYZOf(point2, coordinate2),
						___XYZPointFrom(face, i, coordinate1),
						___XYZPointFrom(face, i, coordinate2),
						___XYZPointFrom(face, (i+1)%___DimOf(face), coordinate1),
						___XYZPointFrom(face, (i+1)%___DimOf(face), coordinate2)))
					return true;
		}
	}
	return false;
}


//if (segment[point1, point2] interset Model)
//return number of intersection points
__host__ __device__
integer IsSegmentIntersectModel(CalcVertex *point1, CalcVertex *point2, CalcMesh *mesh)
{
	integer intersections = 0;
	//DEBUG if 	
//	if ((point2->x == -34.475559)&&(point2->y==143.815369)&&(point2->z==-271.077057))
//	{
//		if (IsSegmentIntersectPolygon(point1, point2, &mesh->Faces[23127]))
//		intersections = 101;
//		else intersections = 100;	
//	}
//	else
	for (integer i=0; i < mesh->NumberOfFaces; i++)
	{
		if(IsSegmentIntersectPolygon(point1, point2, &mesh->Faces[i]))
		{
			intersections++;	
		}
	}
	
	return intersections;
}


//TODO PlaneDefine and SquareDefine - it is a calculate procedure. It shoud be on the GPU or CPU differently from CreateCalcMesh
CalcMesh* CreateCalcMesh(ObjMesh *objMesh)
{
	CalcMesh* calcMesh = NULL;
	if (objMesh != NULL)
	{
		integer i,j;
		calcMesh = (CalcMesh*) malloc(sizeof(CalcMesh));
		assert(calcMesh);
		calcMesh->NumberOfFaces = objMesh->m_iNumberOfFaces;
		calcMesh->NumberOfVertices = objMesh->m_iNumberOfVertices;
		calcMesh->TypesOfFaces = (unsigned char*) calloc(calcMesh->NumberOfFaces, sizeof(unsigned char));
		assert(calcMesh->TypesOfFaces);
		calcMesh->VertexArray = (CalcVertex*) calloc(calcMesh->NumberOfVertices, sizeof(CalcVertex));
		assert(calcMesh->VertexArray);
		
		calcMesh->NumberSphereDetalisation = objMesh->m_iNumberSphereDetalisation;
		calcMesh->SpherePolygonRadiosity = (real*) calloc(calcMesh->NumberSphereDetalisation*(calcMesh->NumberSphereDetalisation-1), sizeof(real));
		assert(calcMesh->SpherePolygonRadiosity);
		for (i=0; i < calcMesh->NumberSphereDetalisation*(calcMesh->NumberSphereDetalisation-1); i++)
		{
			calcMesh->SpherePolygonRadiosity[i] = objMesh->m_aSpherePolygonRadiosity[i];	
		}
		
		calcMesh->Lights = NULL;
		for (i=0; i < calcMesh->NumberOfVertices; i++)
		{
			calcMesh->VertexArray[i].x = objMesh->m_aVertexArray[i].x;
			calcMesh->VertexArray[i].y = objMesh->m_aVertexArray[i].y;
			calcMesh->VertexArray[i].z = objMesh->m_aVertexArray[i].z;
		}
		calcMesh->Faces = (CalcFace*) calloc(calcMesh->NumberOfFaces, sizeof(CalcFace));
		//real p,a,b,c;
		for (i=0; i < calcMesh->NumberOfFaces; i++)
		{
			calcMesh->TypesOfFaces[i] = UNDEFINED_VISION;
			calcMesh->Faces[i].VertexCount = objMesh->m_aFaces[i].m_iVertexCount;
			calcMesh->Faces[i].VertexArray = calcMesh->VertexArray;
			for (j=0; j < calcMesh->Faces[i].VertexCount; j++)
			{
				calcMesh->Faces[i].VertexIndices[j] = objMesh->m_aFaces[i].m_aVertexIndices[j];
			}
			
			PlaneDefine(calcMesh->Faces + i);
			//only for convex polygons
			DefineConvexSquare(calcMesh->Faces+i);
		}
		if (objMesh->m_aLights) //only one light in this implementation
		{
			calcMesh->Lights = (CalcVertex*) malloc(sizeof(CalcVertex));
			calcMesh->Lights[0].x = objMesh->m_aLights[0].x;
			calcMesh->Lights[0].y = objMesh->m_aLights[0].y;
			calcMesh->Lights[0].z = objMesh->m_aLights[0].z;
		}
	}
	return calcMesh;
}

void CopyResults(CalcMesh *calcMesh, ObjMesh *objMesh)
{
	//if (objMesh->m_aTypesOfFaces != NULL)
	{
		integer i;
		for (i=0; i < calcMesh->NumberOfFaces; i++)
		{
			objMesh->m_aTypesOfFaces[i] = calcMesh->TypesOfFaces[i];
		}
		for (i=0; i < calcMesh->NumberSphereDetalisation * (calcMesh->NumberSphereDetalisation-1); i++)
		{
			objMesh->m_aSpherePolygonRadiosity[i] = calcMesh->SpherePolygonRadiosity[i];
		}
	}
}


void DeleteCalcMesh(CalcMesh *calcMesh)
{
	if (calcMesh != NULL)
	{
		int i;
		free(calcMesh->Faces);
		free(calcMesh->VertexArray);
		free(calcMesh->TypesOfFaces);
		if (calcMesh->Lights) free(calcMesh->Lights);
		if (calcMesh->SpherePolygonRadiosity) free(calcMesh->SpherePolygonRadiosity);
		free(calcMesh);
		calcMesh = NULL;
	}
}



//to count first faces from light-point (by segments [light, centroid of polygon])
__host__ __device__
void ToCountFirstFaces(CalcVertex *light, CalcMesh *mesh)
{
	CalcVertex centroid;
	//int temp;
	for(integer i = 0; i < mesh->NumberOfFaces; i++ )
	{
		//for triangles-polygons
		if (mesh->Faces[i].VertexCount == 3)
		{



			GET_CENTROID(centroid.x, centroid.y, centroid.z,
				___XPointFrom(mesh->Faces + i, 0), ___YPointFrom(mesh->Faces + i, 0), ___ZPointFrom(mesh->Faces + i, 0),
				___XPointFrom(mesh->Faces + i, 1), ___YPointFrom(mesh->Faces + i, 1), ___ZPointFrom(mesh->Faces + i, 1),
				___XPointFrom(mesh->Faces + i, 2), ___YPointFrom(mesh->Faces + i, 2), ___ZPointFrom(mesh->Faces + i, 2));
			//the face include centroid => intersections >= 1

			//mesh->TypesOfFaces[i]  = IsSegmentIntersectModel(light, &centroid, mesh
			//temp = IsSegmentIntersectModel(light, &centroid, mesh);
			//if (temp > 10) printf("face %d - intersections %d\n", i, temp);

			DECREASE_SEGMENT_TO_INTERVAL_3D(centroid.x, centroid.y, centroid.z,
				light->x, light->y, light->z);

			if (IsSegmentIntersectModel(light, &centroid, mesh) == 0)
				mesh->TypesOfFaces[i] = FIRST_VISION;
			//else if (IsSegmentIntersectModel(light, &centroid, mesh) == 0)
			//	printf("WTF\n");
			else mesh->TypesOfFaces[i] = OTHER_VISION;
		}
		//else;// if (mesh->Faces[i].VertexCount != 3)
			//printf("ERROR! MORE WHEN 3\n");
    }
}





//__global__
//void cudaIsSegmentIntersectModel(CalcVertex *point1, CalcVertex *point2, CalcMesh *mesh, integer numberOfFace)
//{
//	integer intersections = 0;
//	integer j = threadIdx.x + blockIdx.x * blockDim.x;
//	max j should be more then mesh->NumberOfFaces
//	if (j < mesh->NumberOfFaces)
//	{
//		if(IsSegmentIntersectPolygon(point1, point2, &mesh->Faces[j]))
//		{
//			atomicAdd(&intersections, 1);
//		}
//	}
//
//
//	if (intersections == 0)
//		mesh->TypesOfFaces[numberOfFace] = FIRST_VISION;
//	else mesh->TypesOfFaces[numberOfFace] = OTHER_VISION;
//}


__global__
void cudaToCountFirstFaces(CalcVertex light, CalcMesh* mesh, float* ret)
{
	CalcVertex centroid;
	integer i; //intersections, i, j;
	i = threadIdx.x + blockIdx.x * blockDim.x;// + beginFrom;
	//int temp;
	//mesh->NumberOfFaces=1;
	//for(i=0; i< mesh->NumberOfFaces; i++)
	//if ((i >= beginFrom) && (i < beginFrom + 1))
	if(i < mesh->NumberOfFaces)
	{
		//for triangles-polygons
		if (mesh->Faces[i].VertexCount == 3)
		{

			GET_CENTROID(centroid.x, centroid.y, centroid.z,
				___XPointFrom(mesh->Faces + i, 0), ___YPointFrom(mesh->Faces + i, 0), ___ZPointFrom(mesh->Faces + i, 0),
				___XPointFrom(mesh->Faces + i, 1), ___YPointFrom(mesh->Faces + i, 1), ___ZPointFrom(mesh->Faces + i, 1),
				___XPointFrom(mesh->Faces + i, 2), ___YPointFrom(mesh->Faces + i, 2), ___ZPointFrom(mesh->Faces + i, 2));
			//the face include centroid => intersections >= 1

			//change centroid on delta:
			DECREASE_SEGMENT_TO_INTERVAL_3D(centroid.x, centroid.y, centroid.z,
				light.x, light.y, light.z);

			if (IsSegmentIntersectModel(&light, &centroid, mesh) == 0)
				mesh->TypesOfFaces[i] = FIRST_VISION;
			else mesh->TypesOfFaces[i] = OTHER_VISION;
			
				

//2150 for eleham.obj		
//	if (i==2150) 	{
//		*ret = IsSegmentIntersectModel(&light, &centroid, mesh);
		//*ret = light.z;
//			}

		}
    }
if (i==0) *ret = 99;
//__syncthreads();
}


//to count second faces ONLY AFTER first faces has been defined!
//(by segments [centroid of first face polygon; centroid of not first face polygon])
__host__ __device__
	//	for (integer j=0; j < mesh->NumberOfFaces/(65535*prop.maxThreadsPerBlock)+1; j++) 
	void ToCountSecondAndDoubleFaces(CalcMesh *mesh)
	{
		//just only for triangle-polygons now
		integer i,j;
		CalcVertex centroidFirst, centroidOther, temp;
		//for {all first_vision face polygons}
		for (i=0; i < mesh->NumberOfFaces; i++)
			if ((mesh->TypesOfFaces[i] == FIRST_VISION)&&(mesh->Faces[i].VertexCount == 3))
		{
			GET_CENTROID(centroidFirst.x, centroidFirst.y, centroidFirst.z,
				___XPointFrom(mesh->Faces + i, 0), ___YPointFrom(mesh->Faces + i, 0), ___ZPointFrom(mesh->Faces + i, 0),
				___XPointFrom(mesh->Faces + i, 1), ___YPointFrom(mesh->Faces + i, 1), ___ZPointFrom(mesh->Faces + i, 1),
				___XPointFrom(mesh->Faces + i, 2), ___YPointFrom(mesh->Faces + i, 2), ___ZPointFrom(mesh->Faces + i, 2));

			//fo {all other_vision face polygons}
			for (j=0; j < mesh->NumberOfFaces; j++)
				if ( ( (mesh->TypesOfFaces[j] == OTHER_VISION) || (mesh->TypesOfFaces[j] == FIRST_VISION) )
					&&(mesh->Faces[j].VertexCount == 3))
					{
						GET_CENTROID(centroidOther.x, centroidOther.y, centroidOther.z,
							___XPointFrom(mesh->Faces + j, 0), ___YPointFrom(mesh->Faces + j, 0), ___ZPointFrom(mesh->Faces + j, 0),
							___XPointFrom(mesh->Faces + j, 1), ___YPointFrom(mesh->Faces + j, 1), ___ZPointFrom(mesh->Faces + j, 1),
							___XPointFrom(mesh->Faces + j, 2), ___YPointFrom(mesh->Faces + j, 2), ___ZPointFrom(mesh->Faces + j, 2));

						temp = centroidFirst;
						DECREASE_SEGMENT_TO_INTERVAL_3D(temp.x, temp.y, temp.z,
									centroidOther.x, centroidOther.y, centroidOther.z);
						DECREASE_SEGMENT_TO_INTERVAL_3D(centroidOther.x, centroidOther.y, centroidOther.z,
									temp.x, temp.y, temp.z);

						if (IsSegmentIntersectModel(&temp, &centroidOther, mesh) == 0)
							if (mesh->TypesOfFaces[j] == FIRST_VISION)
								mesh->TypesOfFaces[j] = DOUBLE_VISION;
							else mesh->TypesOfFaces[j] = SECOND_VISION;

					}
		}

		//else if non-triangle ...
		//...
		//..
}







__global__
void cudaToCountSecondAndDoubleFaces(CalcMesh *mesh, float *ret)
{
	//just only for triangle-polygons now
	integer i,j;
	CalcVertex centroidFirst, centroidOther, temp;
	//for {all first_vision face polygons}
	//for (i=0; i < mesh->NumberOfFaces; i++)
	i = threadIdx.x + blockIdx.x * blockDim.x;
	if (i < mesh->NumberOfFaces)
	{
		if ((mesh->TypesOfFaces[i] == FIRST_VISION)&&(mesh->Faces[i].VertexCount == 3))
		{
			GET_CENTROID(centroidFirst.x, centroidFirst.y, centroidFirst.z,
				___XPointFrom(mesh->Faces + i, 0), ___YPointFrom(mesh->Faces + i, 0), ___ZPointFrom(mesh->Faces + i, 0),
				___XPointFrom(mesh->Faces + i, 1), ___YPointFrom(mesh->Faces + i, 1), ___ZPointFrom(mesh->Faces + i, 1),
				___XPointFrom(mesh->Faces + i, 2), ___YPointFrom(mesh->Faces + i, 2), ___ZPointFrom(mesh->Faces + i, 2));

			//fo {all other_vision face polygons}
			for (j=0; j < mesh->NumberOfFaces; j++)
				if ( ( (mesh->TypesOfFaces[j] == OTHER_VISION) || (mesh->TypesOfFaces[j] == FIRST_VISION) )
					&&(mesh->Faces[j].VertexCount == 3))
					{
						GET_CENTROID(centroidOther.x, centroidOther.y, centroidOther.z,
							___XPointFrom(mesh->Faces + j, 0), ___YPointFrom(mesh->Faces + j, 0), ___ZPointFrom(mesh->Faces + j, 0),
							___XPointFrom(mesh->Faces + j, 1), ___YPointFrom(mesh->Faces + j, 1), ___ZPointFrom(mesh->Faces + j, 1),
							___XPointFrom(mesh->Faces + j, 2), ___YPointFrom(mesh->Faces + j, 2), ___ZPointFrom(mesh->Faces + j, 2));

						temp = centroidFirst;
						DECREASE_SEGMENT_TO_INTERVAL_3D(temp.x, temp.y, temp.z,
									centroidOther.x, centroidOther.y, centroidOther.z);
						DECREASE_SEGMENT_TO_INTERVAL_3D(centroidOther.x, centroidOther.y, centroidOther.z,
									temp.x, temp.y, temp.z);

						if (IsSegmentIntersectModel(&temp, &centroidOther, mesh) == 0)
							if (mesh->TypesOfFaces[j] == FIRST_VISION)
								mesh->TypesOfFaces[j] = DOUBLE_VISION;
							else mesh->TypesOfFaces[j] = SECOND_VISION;

					}
		}

		//else if non-triangle ...
		//...
		//..

	}
	*ret = 99;
}




__global__
void cudaToCountSphere(CalcMesh *mesh, float *ret)
{
        integer thread = threadIdx.x + blockIdx.x * blockDim.x;
        if (thread < mesh->NumberSphereDetalisation*(mesh->NumberSphereDetalisation-1))
        {
		integer	  i = thread / mesh->NumberSphereDetalisation + 1,	//for i=1 to i++<N
			  j = thread % mesh->NumberSphereDetalisation,		//for j=0 to j++<N
			  i_ = i-1,
			  j_ = j-1;
		if (j_==-1) j_+=mesh->NumberSphereDetalisation;

		real	  r = 3000.0,
		 	 pi = 3.14159265358979323846,
			 iphi = -pi/2.0 +  i*pi/mesh->NumberSphereDetalisation,
                        i_phi = -pi/2.0 + i_*pi/mesh->NumberSphereDetalisation,
		       jalpha = -pi +  j*2.0*pi/mesh->NumberSphereDetalisation,
		      j_alpha = -pi + j_*2.0*pi/mesh->NumberSphereDetalisation,
			sinjalpha, cosjalpha, siniphi, cosiphi,
			sinj_alpha, cosj_alpha, sini_phi, cosi_phi;
			
			sincos(iphi, &siniphi, &cosiphi);
			sincos(i_phi, &sini_phi, &cosi_phi);
			sincos(jalpha, &sinjalpha, &cosjalpha);
			sincos(j_alpha, &sinj_alpha, &cosj_alpha);
			
		CalcVertex centroid; //the sphere polygon center
		GET_CENTROID_4(	centroid.x, centroid.y, centroid.z, 
				r*cosiphi*cosjalpha, r*cosiphi*sinjalpha, r*siniphi,
				r*cosi_phi*cosjalpha, r*cosi_phi*sinjalpha, r*sini_phi,
				r*cosi_phi*cosj_alpha, r*cosi_phi*sinj_alpha, r*sini_phi,
				r*cosiphi*cosj_alpha, r*cosiphi*sinj_alpha, r*siniphi); 
		
		//change this
		//ToCountFirstFaces(&centroid, mesh);	
		
		mesh->SpherePolygonRadiosity[thread] += 0.5f;
		
		
	}
	*ret = 99;
}



//true, if no polygons between light and polygon
//__host__ __device__
//bool IsPolygonFirstFace(float *point, float *polygon, float *polygon_plane, unsigned int size,
//						  float *ArrayOfPolygon3, float *Polygon3Plane, unsigned int NumberOfPolygon3,
//						  float *ArrayOfPolygon4, float *Polygon4Plane, unsigned int NumberOfPolygon4)
//{
//	bool IntersectedModel=false;
//	int i;
//	for (int j=0; j<size; j++)						//decrease TODO
//	{
//		for (i=0; i<NumberOfPolygon3; i++)
//		{
//			if (polygon == ArrayOfPolygon3+9*i) break;
//			if(IntersectedModel=IsSegmentIntersectPolygon(point, polygon+3*j, ArrayOfPolygon3+9*i, Polygon3Plane+4*i, 3))
//				break;
//		}
//		if(!IntersectedModel)
//			for (i=0; i<NumberOfPolygon4; i++)
//			{
//				if (polygon == ArrayOfPolygon4+12*i) break;
//				if (IntersectedModel = IsSegmentIntersectPolygon(point, polygon+3*j, ArrayOfPolygon4+12*i, Polygon4Plane+4*i, 4))
//					break;
//			}
//		if (IntersectedModel) break;
//	}
//	return IntersectedModel;
//}

__global__ void GPU_tester(CalcMesh* cuda_mesh, float* ret, CalcVertex *light)
{



	//first of all we should to finish copy of our cuda CalcMesh:
	integer i;
	integer id = threadIdx.x + blockIdx.x * blockDim.x;
	//*ret = 2;
 	if (id==0)
		for (i=0; i<cuda_mesh->NumberOfFaces; i++)
		{
			cuda_mesh->Faces[i].VertexArray = cuda_mesh->VertexArray;
		}



}


void GPU_example(CalcMesh* mesh)
{
	//create cudaCalcMesh on GPU
	//i don't know how to use malloc on the gpu, i think that i don't have it on my capability 1.2

	//printf("let's go\n");

	//\F1\E4\E5\EB\E0\F2\FC \ED\EE\F0\EC\E0\EB\FC\ED\F3\FE \EE\E1\F0\E0\E1\EE\F2\EA\F3 \EE\F8\E8\E1\EE\EA

	CalcMesh *cuda_mesh, temp_mesh;

	/*CalcVertex temp_temp, *dev_temp;
	temp_temp.x = 1.1;
	temp_temp.y = 2.2;
	temp_temp.z = 3.13;


	assert( cudaMalloc((void **)&dev_temp, sizeof(CalcVertex)) == cudaSuccess );
	assert( cudaMemcpy(dev_temp, &temp_temp, sizeof(CalcVertex), cudaMemcpyHostToDevice) == cudaSuccess);
	*/


	assert( cudaMalloc((void **)&temp_mesh.VertexArray, mesh->NumberOfVertices*sizeof(CalcVertex)) == cudaSuccess );
	assert( cudaMemcpy(temp_mesh.VertexArray, mesh->VertexArray, mesh->NumberOfVertices*sizeof(CalcVertex), cudaMemcpyHostToDevice) == cudaSuccess);

	temp_mesh.NumberOfFaces = mesh->NumberOfFaces;
	temp_mesh.NumberOfVertices = mesh->NumberOfVertices;
	temp_mesh.NumberSphereDetalisation = mesh->NumberSphereDetalisation;


	assert( cudaMalloc((void **)&temp_mesh.Faces, mesh->NumberOfFaces*sizeof(CalcFace)) == cudaSuccess );
	assert( cudaMemcpy(temp_mesh.Faces, mesh->Faces, mesh->NumberOfFaces*sizeof(CalcFace), cudaMemcpyHostToDevice) == cudaSuccess );
	//after this we need to change all temp_mesh.Faces[ i ].VertexArray to temp_mesh.VertexArray.
	//we are do it in the __global__ function

	assert( cudaMalloc((void **)&temp_mesh.TypesOfFaces, mesh->NumberOfFaces*sizeof(unsigned char)) == cudaSuccess );
	assert( cudaMemcpy(temp_mesh.TypesOfFaces, mesh->TypesOfFaces, mesh->NumberOfFaces*sizeof(unsigned char), cudaMemcpyHostToDevice) == cudaSuccess );

	assert( cudaMalloc((void **)&temp_mesh.Lights, sizeof(CalcVertex)) == cudaSuccess );
	assert( cudaMemcpy(temp_mesh.Lights, mesh->Lights, sizeof(CalcVertex), cudaMemcpyHostToDevice) == cudaSuccess );	
	
	assert( cudaMalloc((void **)&temp_mesh.SpherePolygonRadiosity, mesh->NumberSphereDetalisation*(mesh->NumberSphereDetalisation-1)*sizeof(real)) == cudaSuccess );
        assert( cudaMemcpy(temp_mesh.SpherePolygonRadiosity, mesh->SpherePolygonRadiosity, mesh->NumberSphereDetalisation*(mesh->NumberSphereDetalisation-1)*sizeof(real), cudaMemcpyHostToDevice) == cudaSuccess );
	
	assert( cudaMalloc((void **)&cuda_mesh, sizeof(CalcMesh)) == cudaSuccess );
	assert( cudaMemcpy(cuda_mesh, &temp_mesh, sizeof(CalcMesh), cudaMemcpyHostToDevice) == cudaSuccess );

	//--------------------------------------------------------

	//do something
	//temp
	/*temp_mesh->VertexArray = (CalcVertex*) calloc(mesh->NumberOfVertices, sizeof(CalcVertex));
	memcpy(temp_mesh->VertexArray, mesh->VertexArray, mesh->NumberOfVertices * sizeof(CalcVertex));
	temp_mesh->VertexArray = mesh->VertexArray;
	temp_mesh->Faces = temp_faces;
	temp_mesh->TypesOfFaces = mesh->TypesOfFaces;
*/

	//		CalcVertex light;
	//light.x = 100.0f;
	//light.y = light.z = 50.0f;


	//ToCountFirstFaces(&light, temp_mesh);

	float *temp, temp2;
	cudaMalloc((void**)&temp, sizeof(float));


	cudaDeviceProp prop;
	cudaGetDeviceProperties( &prop , 0 );
	//printf("%d\n", prop.maxThreadsPerBlock);

	//CalcVertex light;
	//light.x = mesh->Lights[0].x;
	//light.y = mesh->Lights[0].y;
	//light.z = mesh->Lights[0].z;
	//DWORD time;

	GPU_tester<<<1, 1>>>(cuda_mesh, temp, mesh->Lights);

	{
		printf("DEBUG: cudaToCountFirstFaces() on %i x %i \n ",60000,prop.maxThreadsPerBlock-150);
		cudaToCountFirstFaces<<< 60000, prop.maxThreadsPerBlock-150>>>(mesh->Lights[0], cuda_mesh, temp);
		//for (integer face = 0; face < 1000; face++){
		//cudaToCountFirstFaces<<< 1, 1>>>(face, light, cuda_mesh, temp);
		printf("%s\n",cudaGetErrorString(cudaThreadSynchronize()));	
		cudaMemcpy(&temp2, temp, sizeof(float), cudaMemcpyDeviceToHost);
		printf("DEBUG: it's resulsts %f \n", temp2);
		//}
		
		printf("DEBUG: cudaToCountSecondAndDoubleFaces() on %i x %i \n ",60000,prop.maxThreadsPerBlock-150);
		cudaToCountSecondAndDoubleFaces<<< 50000, prop.maxThreadsPerBlock-150>>>(cuda_mesh, temp);
		printf("%s\n",cudaGetErrorString(cudaThreadSynchronize()));
		cudaMemcpy(&temp2, temp, sizeof(float), cudaMemcpyDeviceToHost);
		printf("GPU test 2:%f \n", temp2);
		
		printf("DEBUG: cudaToCountSphere() on %i x %i \n ",60000,prop.maxThreadsPerBlock-150);
                cudaToCountSphere<<< 50000, prop.maxThreadsPerBlock-150>>>(cuda_mesh, temp);
                printf("%s\n",cudaGetErrorString(cudaThreadSynchronize()));
                cudaMemcpy(&temp2, temp, sizeof(float), cudaMemcpyDeviceToHost);
                printf("GPU test 3:%f \n", temp2);	
		
	}

	cudaMemcpy(mesh->TypesOfFaces, temp_mesh.TypesOfFaces, mesh->NumberOfFaces*sizeof(unsigned char), cudaMemcpyDeviceToHost);
	cudaMemcpy(mesh->SpherePolygonRadiosity, temp_mesh.SpherePolygonRadiosity, mesh->NumberSphereDetalisation*(mesh->NumberSphereDetalisation-1)*sizeof(real), cudaMemcpyDeviceToHost);
	
//DEBUG
	{
/*		CalcVertex point2;
BUG Ham		point2.x = -34.475559;
		point2.y = 143.815369;
		point2.z = -271.077057;
		if (IsSegmentIntersectPolygon(&light, &point2, &(mesh->Faces[23127]))) printf("Yes, intersection\n");
		else printf("No intersection\n");
*/	
		//ON_ONE_SIDE(point2.x, point2.y,
		//	 p2x, p2y,
		//	 p1x_line, p1y_line,
		//	 p2x_line, p2y_line);
		//printf("%f\n", mesh->Faces[23127].VertexArray[ mesh->Faces[23127].VertexIndices[2] ].z);	
	}

	int j;
	//from 2000 to 2200 - good faces for experiments in  ham
	//for (j=800; j<1000; j++)		
	//mesh->TypesOfFaces[2150] = 2;
	//mesh->TypesOfFaces[23127] = 2;

/*CPU	{
	//time = GetTickCount();
		ToCountFirstFaces(&light, mesh);
		ToCountSecondAndDoubleFaces(mesh);		
	//time = time - GetTickCount();
//	printf("time: %i\n", time);
	}
*/


	//--------------------------------------------------------
	//free memory
	cudaFree(cuda_mesh);
	cudaFree(temp_mesh.TypesOfFaces);
	cudaFree(temp_mesh.Faces);
	cudaFree(temp_mesh.VertexArray);


}
