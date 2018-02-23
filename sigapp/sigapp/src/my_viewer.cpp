
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>
#include <chrono>
#include <ctime>
# include <sigogl/ws_run.h>

static bool keeprunning = false; 
static float r1dt = 0.0;
static float r2dt = 0.0;
static double seconds = 0.0;




static GsVec lighting = GsVec(0.01f, 0.15f, 0.4f);

static GsMat shadowXZ = GsMat(1.0f, -lighting.x/lighting.y, 0.0f, 0.0f, 
									0.0f, 0.0f, 0.0f, 0.0f, 
									0.0f, -lighting.z/lighting.y, 1.0f, 0.0f, 
									0.0f, 0.0f, 0.0f, 1.0f );
static GsMat shadowYZ = GsMat(0.0f, 0.0f, 0.0f, 0.0f, 
								   -lighting.y/lighting.x, 1.0f, 0.0f, 0.0f, 
								   -lighting.z/lighting.x, 0.0f, 1.0f, 0.0f, 
									0.0f, 0.0f, 0.0f, 1.0f);
static GsMat shadowXY = GsMat(1.0f, 0.0f, -lighting.x/lighting.z, 0.0f,
									0.0f, 1.0f, -lighting.y/lighting.z, 0.0f,
									0.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_clock_scene ();
}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );
	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}



void MyViewer::build_clock_scene() {
	
	
	//Declare matrices

	rootg()->remove_all();

	SnGroup *clock = new SnGroup;
	SnGroup *clockShadow = new SnGroup;

	GsMat m;

	GsMat rotM; 


	//Declared the parts of a clock
	SnPrimitive *clockbody; 


	clockbody = new SnPrimitive(GsPrimitive::Cylinder, 1.0f, 1.0f, 0.01f);
	clockbody->prim().material.diffuse = GsColor::red;
	clockbody->prim().center = GsPnt(0.0f, 0.0f, 0.0f);

	SnPrimitive *minuteHand; 

	minuteHand = new SnPrimitive(GsPrimitive::Capsule, 0.05f, 0.05f, 0.45f);
	minuteHand->prim().material.diffuse = GsColor::lightblue; 
	minuteHand->prim().center = GsPnt(0.0f, 0.5f, 0.0f);

	SnPrimitive *hourHand; 

	hourHand = new SnPrimitive(GsPrimitive::Capsule, 0.05f, 0.05f, 0.3f);
	hourHand->prim().material.diffuse = GsColor::green;
	hourHand->prim().center = GsPnt(0.0f, 0.3f, 0.0f);
	
	//Put clockbody in scence graph

	SnGroup *g1 = new SnGroup;
	g1->separator(true);
	g1->add(_t1 = new SnTransform);
	g1->add((clockbody));
	rotM.rotx(GS_TORAD(90.0f));
	_t1->get().mult(m, rotM);


	//Put minutehand in scene graph

	SnGroup *g2 = new SnGroup;
	g2->separator(true);
	g2->add(_t2 = new SnTransform);
	g2->add((minuteHand));
	
	//Put hourhand in scene graph

	SnGroup *g3 = new SnGroup;
	g3->separator(true);
	g3->add(_t3 = new SnTransform);
	g3->add((hourHand));

	clock->add(g1);
	clock->add(g2);
	clock->add(g3);

	clockShadow->separator(true);
	clockShadow->add(_t4 = new SnTransform);
	clockShadow->add(clock);
	rotM.rotx(GS_TORAD(0.0f));
	m.translation(0.0f, -1.0f, -2.6f);
	_t4->get().mult(m, shadowXZ);

	rootg()->add(clock);
	rootg()->add(clockShadow);
	//rootg()->add(g2);
	//rootg()->add(g3);


}

void MyViewer::clock_animation() {


	if (_animating) return;
	_animating = true; 

	
	GsMat xRot; 
	GsMat yRot; 
	GsMat zRot; 
	GsMat m1;
	GsMat m2; 	
	
	//Records the current time
	auto start = std::chrono::system_clock::now();
	
	
	while(keeprunning)
	{
		//Captures the value of the updated time
		auto end = std::chrono::system_clock::now();
		
		//Records the difference in time
		std::chrono::duration<double> elapsed_seconds = end - start;

		//Checks if 60 seconds had passed
		if (seconds >= 60.0f) {
			zRot.rotz(GS_TORAD(r2dt -= 6.0f));
			_t3->get().mult(m1, zRot);
			seconds = 0.0;
		}
		//Checks if 1 second had passed
		if (elapsed_seconds.count() >= 1.0) {
			//Increments the seconds value by at least 1.0
			seconds += elapsed_seconds.count();
			zRot.rotz(GS_TORAD(r1dt -= 6.0f));
			_t2->get().mult(m2, zRot);
			//resets the current time to the most updated value
			start = end;
		}
		
		
		render();
		ws_check();
	} 
	

	

	_animating = false;
}

void MyViewer::reset_animation() {

	GsMat m1; 
	GsMat m2; 

	GsMat zRot;

	seconds = 0.0;
	r1dt = 0.0;
	r2dt = 0.0;

	zRot.rotz(GS_TORAD(0));
	_t2->get().mult(m1, zRot);
	
	
	zRot.rotz(GS_TORAD(0));
	_t3->get().mult(m1, zRot);
	 

	render();
	
}

void MyViewer::build_scene ()
{
	SnPrimitive* p;

	p = new SnPrimitive(GsPrimitive::Box,1,3,1);
	p->prim().material.diffuse=GsColor::yellow;
	add_model ( p, GsVec(0,0,0) );

	p = new SnPrimitive(GsPrimitive::Sphere,2);
	p->prim().material.diffuse=GsColor::red;
	add_model ( p, GsVec(-4,0,0) );

	p = new SnPrimitive(GsPrimitive::Cylinder,1.0,1.0,1.5);
	p->prim().material.diffuse=GsColor::blue;
	add_model ( p, GsVec(4,0,0) );

	p = new SnPrimitive(GsPrimitive::Capsule,1,1,3);
	p->prim().material.diffuse=GsColor::red;
	add_model ( p, GsVec(8,0,0) );

	p = new SnPrimitive(GsPrimitive::Ellipsoid,2.0,0.5);
	p->prim().material.diffuse=GsColor::green;
	add_model ( p, GsVec(-8,0,0) );
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	
	int ind = gs_random ( 0, rootg()->size()-1 ); // pick one child
	SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		double yinc = (t-lt)*v;
		if ( t>2 ) yinc=-yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if ( m.e24<0 ) m.e24=0; // make sure it does not go below 0
		manip->initial_mat ( m );
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while ( m.e24>0 );
	_animating = false;
}

void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		case 'q':
		{
			GsMat m; 
			lighting.x += 0.1f;

			shadowXZ = GsMat(1.0f, -lighting.x / lighting.y, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -lighting.z / lighting.y, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m.translation(0.0f, -1.0f, -2.6f);

			_t4->get().mult(m, shadowXZ);

			render();

			return 1; 
		}
		case 'a':
		{
			GsMat m;
			lighting.x -= 0.1f;

			shadowXZ = GsMat(1.0f, -lighting.x / lighting.y, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -lighting.z / lighting.y, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m.translation(0.0f, -1.0f, -2.6f);

			_t4->get().mult(m, shadowXZ);

			render();

			return 1; 
		}
		case 'w':
		{

			GsMat m;
			lighting.y += 0.1f;

			shadowXZ = GsMat(1.0f, -lighting.x / lighting.y, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -lighting.z / lighting.y, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m.translation(0.0f, -1.0f, -2.6f);

			_t4->get().mult(m, shadowXZ);

			render();
			return 1; 
		}
		case 's':
		{
			GsMat m;
			lighting.y -= 0.1f;

			shadowXZ = GsMat(1.0f, -lighting.x / lighting.y, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -lighting.z / lighting.y, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m.translation(0.0f, -1.0f, -2.6f);

			_t4->get().mult(m, shadowXZ);

			render();
			return 1; 
		}
		case 'e': {
			GsMat m;
			lighting.z += 0.1f;

			shadowXZ = GsMat(1.0f, -lighting.x / lighting.y, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, -lighting.z / lighting.y, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			m.translation(0.0f, -1.0f, -2.6f);

			_t4->get().mult(m, shadowXZ);

			render();
		}
		case GsEvent::KeySpace:
		{

			if (keeprunning == false) {
				keeprunning = true;
				clock_animation();
				
			}
			else {
				keeprunning = false;
				clock_animation();
			}
			return 1; 
		}
		case GsEvent::KeyEnter:
		{
			keeprunning = false;
			clock_animation();

			reset_animation();

			return 1;
		}
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
