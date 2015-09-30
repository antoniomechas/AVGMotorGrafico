#pragma once
#include "ofMain.h"
#include "ofxFilterLibrary.h"
#include "ofxGui.h"

class EffectsManager
{

	public:

		void						setup				( int width, int height, ofPoint guiPos = ofPoint(10,10) );
		void						drawWithFilter		( ofTexture & texIn );
		void						drawGui				( );
		int 						getNumActiveFilters ( );
		void						saveGui				( string guiFileName );
		void						loadGui				( string guiFileName );

	private:

		void						toggleFilterPressed	( bool & value );
		void						setupFilters		( );
		void						setupGui			( );
		void						createFilterChain	( );

		int							width;
		int							height;

		ofFbo						_fboOut;

		FilterChain					*fc;
		vector<string>				filterNames;
		vector<ofxToggle *>			filterToggles;
		vector<AbstractFilter *>    _filters;

		ofxPanel					guiFilters;
		//ofxPanel					guiFiltersParams;
		ofPoint						_guiPos;

		ofxFloatSlider				paramLowPass;
		ofxIntSlider				paramKuwahara;
		ofxFloatSlider				paramEmboss;

		bool						bLoading;		//para desactivar los listeners del menu, sino los ejecuta varias veces al cargar el preset

};