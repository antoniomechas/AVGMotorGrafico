#include "EffectsManager.h"

//--------------------------------------------------------------
//
void EffectsManager::setup (int w, int h, ofPoint guiPos)
{
	width = w;
	height = h;
	
	_guiPos = guiPos;

	ofDisableArbTex();
	//_fboOut.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA32F);
 
	setupFilters();

	ofEnableArbTex();
	
	setupGui();

	bLoading = false;

}

//--------------------------------------------------------------
//
void EffectsManager::setupGui(){


	guiFilters.setup("","", _guiPos.x, _guiPos.y);
	guiFilters.setName("filtros");

	for (int i = 0 ; i < filterNames.size() ; i++)
	{
		filterToggles.push_back( new ofxToggle );
		filterToggles.back()->addListener(this, &EffectsManager::toggleFilterPressed);
		guiFilters.add(filterToggles.back()->setup(filterNames[i],false));
	}

	//guiFilters.setup("","", guiFilters.getPosition().x + guiFilters.getWidth() + 20, _guiPos.y);
	//guiFilters.setName("filter Params");
	guiFilters.add(paramKuwahara.setup("kuwahara",6, 1, 20));
	guiFilters.add(paramEmboss.setup("emboss",2.0, 0, 10));
	guiFilters.add(paramLowPass.setup("lowPass",0.9, 0, 1.0));

}

//--------------------------------------------------------------
//
void EffectsManager::setupFilters()
{
	filterNames.push_back("Halftone");
	filterNames.push_back("Crosshatch");
	filterNames.push_back("Kuwahara");
	filterNames.push_back("SobelEdgeDetection");
	filterNames.push_back("Bilateral");
	filterNames.push_back("Sketch");
	filterNames.push_back("Dilation");
	filterNames.push_back("PerlinPixellation");
	filterNames.push_back("XYDerivative");
	filterNames.push_back("ZoomBlur");
	filterNames.push_back("Emboss");
	filterNames.push_back("SmoothToon");
	filterNames.push_back("TiltShift");
	filterNames.push_back("Voronoi");
	filterNames.push_back("CGAColorspace");
	filterNames.push_back("Erosion");
	filterNames.push_back("Vignette");
	filterNames.push_back("Posterize");
	filterNames.push_back("Laplacian");
	filterNames.push_back("Pixelate");
	filterNames.push_back("HarrisCornerDetection");
	filterNames.push_back("MotionDetection");
	filterNames.push_back("LowPass");

	//chains
	filterNames.push_back("charcoal");
	filterNames.push_back("foggedTexturedGlass");
	filterNames.push_back("WaterColor");

	fc = new FilterChain(width, height, "fc");
}


//--------------------------------------------------------------
//
void EffectsManager::toggleFilterPressed ( bool & value )
{
	if (!bLoading)
		createFilterChain();
}

//--------------------------------------------------------------
//
void EffectsManager::createFilterChain ( )
{
	ofDisableArbTex();

	_filters.clear();
	
	fc->_filters.clear();
	
	bool bHay = false;

	for (int i = 0 ; i < filterToggles.size() ; i++)
	{
		int b = *filterToggles[i];
		if (b == 1)
		{
			cout << "Añadido filtro : " << filterNames[i] << endl;
			bHay = true;
			switch (i)
			{
				case 0: //Halftone
					fc->addFilter(new HalftoneFilter(width, height, 0.001));
					break;

				case 1:
					 fc->addFilter(new CrosshatchFilter(width, height));
					 break;

				case 2:
					 fc->addFilter(new KuwaharaFilter(paramKuwahara));
					 break;
			
				case 3:
					 fc->addFilter(new SobelEdgeDetectionFilter(width, height));
					 break;

				case 4:
					 fc->addFilter(new BilateralFilter(width, height));
					 break;

				case 5:
					 fc->addFilter(new SketchFilter(width, height));
					 break;

				case 6:
					 fc->addFilter(new DilationFilter(width, height));
					 break;

				case 7:
					 fc->addFilter(new PerlinPixellationFilter(width, height));
					 break;

				case 8:
					 fc->addFilter(new XYDerivativeFilter(width, height));
					 break;

				case 9:
					 fc->addFilter(new ZoomBlurFilter());
					 break;

				case 10:
					 fc->addFilter(new EmbossFilter(width, height, paramEmboss));
					 break;

				case 11:
					 fc->addFilter(new SmoothToonFilter(width, height));
					 break;

				case 12:
					// fc->addFilter(new TiltShiftFilter(width, height));
					 break;

				case 13:
					// fc->addFilter(new VoronoiFilter(width, height));
					 break;

				case 14:
					 fc->addFilter(new CGAColorspaceFilter());
					 break;

				case 15:
					 fc->addFilter(new ErosionFilter(width, height));
					 break;

				case 16:
					 fc->addFilter(new VignetteFilter());
					 break;

				case 17:
					 fc->addFilter(new PosterizeFilter(8));
					 break;

				case 18:
					 fc->addFilter(new LaplacianFilter(width, height, ofVec2f(1, 1)));
					 break;

				case 19:
					 fc->addFilter(new PixelateFilter(width, height));
					 break;
			
				case 20:
					 //fc->addFilter(new HarrisCornerDetectionFilter(width, height));
					 break;
			
				case 21:
					//fc->addFilter(new MotionDetectionFilter(width, height));
					 break;
			
				case 22:
					 fc->addFilter(new LowPassFilter(width, height, paramLowPass));
					 break;

				case 23: //charcoal
					fc->addFilter(new BilateralFilter(width, height, 4, 4));
					fc->addFilter(new GaussianBlurFilter(width, height, 2.f ));
					fc->addFilter(new DoGFilter(width, height, 12, 1.2, 8, 0.99, 4));
					break;

				case 24: //foggyTexturedGlass
					fc->addFilter(new PerlinPixellationFilter(width, height, 13.f));
					fc->addFilter(new EmbossFilter(width, height, 0.5));
					fc->addFilter(new GaussianBlurFilter(width, height, 3.f));
					break;

				case 25: //waterColor
					fc->addFilter(new KuwaharaFilter(9));
					fc->addFilter(new LookupFilter(width, height, "img/lookup_miss_etikate.png"));
					fc->addFilter(new BilateralFilter(width, height));
					fc->addFilter(new PoissonBlendFilter("img/canvas_texture.jpg", width, height, 2.0));
					fc->addFilter(new VignetteFilter());
			}
		}
	}
	
	if (bHay)
	{
		cout << "Número de filtros en el filter Chain: " << fc->_filters.size() << endl;
		_filters.push_back(fc);
	}

	ofEnableArbTex();

}

//--------------------------------------------------------------
//
void EffectsManager::drawWithFilter( ofTexture & texIn)
{
	if (_filters.size() > 0)
	{
		ofDisableArbTex();
		ofPushMatrix();
		//ofScale(-1, 1);
		//ofTranslate(-ofGetWidth(), 0);
		_filters[0]->begin();
		texIn.draw(0,0);
		_filters[0]->end();
		ofPopMatrix();
		ofEnableArbTex();
	}
	else
		texIn.draw(0,0);
}

//--------------------------------------------------------------
//
void EffectsManager::drawGui()
{
	guiFilters.draw();
	//guiFiltersParams.draw();
}

int EffectsManager::getNumActiveFilters()
{
	return fc->_filters.size();
}

void EffectsManager::saveGui( string fileName )
{
	guiFilters.saveToFile(fileName);
}

void EffectsManager::loadGui( string fileName )
{
	bLoading = true;
	guiFilters.loadFromFile(fileName);
	bLoading = false;
	createFilterChain();
}
