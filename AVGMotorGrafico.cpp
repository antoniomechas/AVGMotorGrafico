//2
//3 - desde el PC, en el servidor tiene que haber un //5 desde el portatil
// desde el portatil
#include "AVGMotorGrafico.h"

void AVGMotorGrafico::setup(int w, int h)
{
	width = w;
	height = h;

	fbo.allocate(w, h, GL_RGBA32F);
	fboTex.allocate(w, h, GL_RGBA32F);
	fbo2.allocate(w, h, GL_RGBA32F);
	fboOut.allocate(w, h, GL_RGBA32F);
	ofDisableArbTex();
	fboNoARB.allocate(w, h, GL_RGBA32F);
	ofEnableArbTex();
	effectsManager.setup(w, h);

//	texMask.allocate(w, h, GL_LUMINANCE);

	//imagenOrigen.allocate(w,h);
	//imagenDestino.allocate(w,h);
	//imagenFondo.allocate(w,h);
	//textura.allocate(w,h);
	//imagenTmp.allocate(w,h);
//	imgTmp.allocate(w,h,OF_IMAGE_COLOR);

//	tOrigen = ORIGEN_CAPTURA_DRAW;
	//bUsaShader = true;
	//bOn = false; //Esta desactivado por defecto
	//bClipping = true;
	//bUsaImgFondo = false;
	//iShader = 0;

	//loadImage(&imagenFondo, "data/images/tanque.jpg");
	//loadImage(&textura, "shaders/texturas/tex01.jpg");

	iIntervaloMax = 0;
	iIntervaloMin = 0;
	lastTimeChecked = ofGetElapsedTimeMillis();
	lastTimeInterval = ofGetElapsedTimeMillis();

	shaderI.load("shaders/invertir");		//  Se carga el shader que se encarga de invertir

//	maskType = MASK_TYPE_IMG;	// Para darle un valor por defecto

	setupPingPong();

	//Parametros del contorno que se le pasará al shader
	shaderCenter = ofVec2f(0.5, 0.5);
	shaderWidth = 0.5;
	shaderHeight = 0.5;

	bDontDraw = false;

	//ofxPostProcessing
	post.init(w, h);
	post.createPass<BloomPass>()->setEnabled(false);
    post.createPass<FxaaPass>()->setEnabled(false);

	bClipping = false; //esto ya no se utiliza con la llegada del resolume
}



void AVGMotorGrafico::setupPingPong( void )
{
	ofEnableArbTex();
	//pingPong[0].allocate(width, height, GL_RGB);
	//pingPong[1].allocate(width, height, GL_RGB);
	pingPong[0].allocate(width, height, GL_RGBA32F_ARB);
	pingPong[1].allocate(width, height, GL_RGBA32F_ARB);
	//pingPong[0].allocate(width, height, GL_RGBA);
	//pingPong[1].allocate(width, height, GL_RGBA);
    for(int i = 0; i < 2; i++)
	{
		pingPong[i].begin();
			ofClear(0,255);
        pingPong[i].end();
	}

	src = &pingPong[0];
	dst = &pingPong[1];

	cout << endl << "Cargando shader damping / alpha" << endl;
	shaderPingPong.load("shaders/pintura");
}


void AVGMotorGrafico::draw( void )
{
	// Si no está activo, no tiene nada que hacer
	if (!bOn)
		return;

	ofPushStyle();

	if (bUsaShader)
	{
		//  Lo hará cada MILIS_CHECK
		if (ofGetElapsedTimeMillis() - lastTimeChecked > MILIS_CHECK)
		{
			setNombreShader();
			string frag = "shaders/" + nombreShader + ".frag";
			string vert = "shaders/" + nombreShader + ".vert";

			ofFile fragFile(frag), vertFile(vert);
			Poco::Timestamp fragTimestamp = fragFile.getPocoFile().getLastModified();
			Poco::Timestamp vertTimestamp = vertFile.getPocoFile().getLastModified();
			if(fragTimestamp != lastFragTimestamp || vertTimestamp != lastVertTimestamp)
			{
				shader1.load("shaders/" + nombreShader);
				//shader2.load("shaders/shader2");
			}

			lastFragTimestamp = fragTimestamp;
			lastVertTimestamp = vertTimestamp;
		}
	}

	ofDisableAlphaBlending();
	
	if (bUsaShader)
	{

		fbo.begin();
			drawShader();
		fbo.end();
		nextFbo = &fbo;
	}
	else	
		nextFbo = &fboTex;

	// En nextFbo tenemos el resultado hasta ahora de la imagen resultante
	// en el caso de que el ping pong esté habilitado, lo utilizamos sobre el fbo actual
	// el shader ping pong le aplicara un blur segun el parámetro kernelSize, y un alpha damping
	// TODO: incluir el invertir en este shader también, para matar los 3 pajaros en un shader
	
	drawPingPong(nextFbo);

	if (effectsManager.getNumActiveFilters() > 0)
	{
		fboNoARB.begin();
			nextFbo->draw(0,0);
		fboNoARB.end();

		fbo2.begin();
			effectsManager.drawWithFilter(fboNoARB.getTextureReference());
		fbo2.end();
		nextFbo = &fbo2;
	}


	if (bPostBloom || bPostFxaa)
	{
		fbo.begin();
			post[0]->setEnabled(bPostBloom);
			post[1]->setEnabled(bPostFxaa);
			post.begin();
				nextFbo->draw(0,0,width,height);
			post.end();
		fbo.end();
		nextFbo = &fbo;
	}

	drawInvertir(nextFbo);
	
	//fbo2.draw(0,0);
	
	nextFbo->draw(0,0); 

	//if (bClipping)
	//	drawClipping();

	ofPopStyle();

}

//	
//
void AVGMotorGrafico::drawInvertir(ofFbo *srcFbo)
{
	bool bDoInvertir = false;

	if (bInvertir && !bFlash)
		bDoInvertir = true;

	if (!bInvertir && !bFlash)
		bDoInvertir = false;

	if (bFlash)
	{
		if ((iIntervaloMin == 0) && (iIntervaloMax == 0))
			bDoInvertir = true;
		else
		{
			if ((ofGetElapsedTimeMillis() - lastTimeInterval) > ofRandom(iIntervaloMin,iIntervaloMax))
			{
				bDoInvertir = !bDoInvertir;
				lastTimeInterval = ofGetElapsedTimeMillis();
			}
		}
	}

	if (bDoInvertir)
	{
		fbo2.begin();
			shaderI.begin();
			shaderI.setUniformTexture("tex0", srcFbo->getTextureReference(),  1);
			renderDummy();
			shaderI.end();
		fbo2.end();
		nextFbo = &fbo2;
	}
}


//
//	Dibuja en el FBO de ping pong para hacerle un blur y alpha damping y mantener el resultado
//
void AVGMotorGrafico::drawPingPong(ofFbo *srcFbo)
{
	
	if (bUsePingPong)
	{
		ofSetColor(255,255,255);

		ofEnableAlphaBlending();	

		src->begin();
			srcFbo->draw(0,0);
		src->end();

		ofDisableAlphaBlending();

		dst->begin();
			//ofClear(0, 0, 0, 0);
			shaderPingPong.begin();
				shaderPingPong.setUniformTexture("tex0", src->getTextureReference(), 1);
				shaderPingPong.setUniform1i("kernelSize", iKernelSize);
				shaderPingPong.setUniform1f("alphaDamping", fPingPongAlphaDamping);
				renderDummy();
			shaderPingPong.end();
		dst->end();

		swap(src, dst);

		nextFbo = src;
	}

}

//
//	Dibuja la escena con el shader correspondiente
//
void AVGMotorGrafico::drawShader()
{
	shader1.begin();

		shader1.setUniform2f("resolution",width, height);
		//shader1.setUniform2f("mouse",(float)iShaderX / (float)width, (float)iShaderY / (float)height);
		shader1.setUniform1f("time", ofGetElapsedTimef() );
		//shader1.setUniform1f("volumen", volumen );
		shader1.setUniform1f("alphaShader", fAlphaShader );
		
		switch(iShader)
		{
			case 1:	// Tunnel
				//cout << "centro: " << shaderCenter << endl;
				shader1.setUniform2f("centro", shaderCenter.x, shaderCenter.y);
				shader1.setUniform1f("width", shaderWidth);
				shader1.setUniform1f("height", shaderHeight);
				break;

			case 2: //flare
				shader1.setUniform2f("center", shaderCenter.x, shaderCenter.y);
				break;
			
			case 3:
				//textura.draw(0,0);
				shader1.setUniform4f("iMouse",shaderCenter.x, shaderCenter.y, 1.0f, 1.0f);
				shader1.setUniformTexture("tex0", fboTex.getTextureReference(),  1);
				break;

			case 4: //voronoi
				//cout << ofNoise(ofGetElapsedTimef()) << endl;
				shader1.setUniformTexture("tex0", fboTex.getTextureReference(),  1);
				shader1.setUniform1f("density", 120.0f * ofNoise(ofGetElapsedTimef()));
				break;

			case 5: //blur with mask
				//fboTex.draw(0,0);
				shader1.setUniform1i("kernelSize", iKernelSize);
				shader1.setUniformTexture("tex0", fboTex.getTextureReference(),  1);
				//if (maskType == MASK_TYPE_IMG)
				//	shader1.setUniformTexture("tex1", texMask,  2);
				//else
				//	shader1.setUniformTexture("tex1", *texMaskExterna,  2);
				break;

			default:
				shader1.setUniform4f("iMouse",shaderCenter.x, shaderCenter.y, 1.0f, 1.0f);
				shader1.setUniformTexture("tex0", fboTex.getTextureReference(),  1);
		}
		
		renderDummy();	//Para que el shader se active	
	
	shader1.end();

}


//
//  Utiliza la imagen de fondo como base y le aplica efectos según lo que hay en imagenDestino
//
//void AVGMotorGrafico::aplicaEfectoSobreFondo()
//{
//	int p = 0;
//	float f;
//	unsigned char *c = imagenFondo.getPixels();
//	unsigned char *d = imagenDestino.getPixels();
//	//voy a suponer que los pixeles estan en el rango 0-255
//	/*
//	for (int i = 0 ; i < imagenFondo.width ; i++)
//		for (int k = 0 ; k < imagenFondo.height ; k++)
//		{
//			for (int h = 0; h < 3; h++)
//			{
//				f = (float)imagenFondo.getPixels()[p] * ((float)imagenDestino.getPixels()[p] / 255.0f);
//				imagenDestino.getPixels()[p] = (unsigned char)f;
//				p++;
//
//			}
//		}
//		//cvAnd(imagenDestino.getCvImage(), imagenFondo.getCvImage(), imagenTmp.getCvImage());
//	*/
//	for (int i = 0 ; i < imagenFondo.width * imagenFondo.height * 3 ; i++)
//	{
//		f = (float)c[i] * ((float)d[i] / 255.0f);
//		//f = c[i];
//		//d[i] = c[i];
//		d[i] = (unsigned char)f;
//	}
//
//	//imagenDestino = imagenTmp;
//}


void AVGMotorGrafico::drawClipping()
{
	//Clipping
	//ofSetColor(0,0,0);
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POLYGON_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );

	glColor3f(0.0f,0.0f,0.0f);
	ofFill();
	
	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(ptoClipping[0].x, ptoClipping[0].y);
		glVertex2f(ptoClipping[3].x, ptoClipping[3].y);
		glVertex2f(0,height);
	glEnd();

	glBegin(GL_QUADS);
		glVertex2f(0,0);
		glVertex2f(width,0);
		glVertex2f(ptoClipping[1].x, ptoClipping[1].y);
		glVertex2f(ptoClipping[0].x, ptoClipping[0].y);
	glEnd();

	glBegin(GL_QUADS);
		glVertex2f(ptoClipping[1].x, ptoClipping[1].y);
		glVertex2f(width,0);
		glVertex2f(width,height);
		glVertex2f(ptoClipping[2].x, ptoClipping[2].y);
	glEnd();

	glBegin(GL_QUADS);
		glVertex2f(ptoClipping[3].x, ptoClipping[3].y);
		glVertex2f(ptoClipping[2].x, ptoClipping[2].y);
		glVertex2f(width,height);
		glVertex2f(0,height);
	glEnd();

	ofNoFill();
	
	glDisable( GL_LINE_SMOOTH );
	glDisable( GL_POLYGON_SMOOTH );
	glDisable( GL_BLEND );
}

//void AVGMotorGrafico::update( ofxCvGrayscaleImage *img )
//{
//
//	imagenOrigen = *(img);
//
//}

//void AVGMotorGrafico::setImagenOrigen( tipoOrigen t)
//{
//	tOrigen = t;
//}

// OJO: si en el gui se cambia que haya o no ping pong justo entre captura.begin y captura.end -> incongruencia
void AVGMotorGrafico::beginCaptureDraw()
{
	if (!bOn)
		return;

	ofPushStyle();
	ofPushMatrix();
	//if (bUsePingPong)
	//	src->begin();
	//else
	//	fboTex.begin();
	//ofClear(0,0);
	fboTex.begin();
		ofClear(0,1);
		//ofBackground(backgroundColor);
}

void AVGMotorGrafico::endCaptureDraw()
{
	if (!bOn)
		return;
	
	//if (bUsePingPong)
	//{
	//	src->end();
	//	// copio src en fboTex para que utilice siempre este FBO en los shaders a continuación
	//	//fboTex = *src;
	//	ofEnableAlphaBlending();
	//	//ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//	fboTex.begin();
	//		src->draw(0,0);
	//	fboTex.end();
	//	ofDisableAlphaBlending();
	//}
	//else
	//	fboTex.end();

	fboTex.end();

	ofPopMatrix();
    ofPopStyle();	
}

void AVGMotorGrafico::renderDummy()
{
	ofSetColor(255,255,255);
	glBegin(GL_QUADS);
    //glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    //glTexCoord2f(width, 0); glVertex3f(width, 0, 0);
    //glTexCoord2f(width, height); glVertex3f(width, height, 0);
    //glTexCoord2f(0,height);  glVertex3f(0,height, 0);
    glTexCoord2f(0, 0); glVertex3f(0, 0, 0);
    glTexCoord2f(1.0f, 0); glVertex3f(width, 0, 0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(width, height, 0);
    glTexCoord2f(0,1.0f);  glVertex3f(0,height, 0);
    glEnd();
}

void AVGMotorGrafico::setNombreShader()
{
	switch (iShader)
	{
		case 0:
			nombreShader = "generaColor";
			break;

		case 1:
			nombreShader = "tunnel_purple";
			break;

		case 2:
			nombreShader = "flare";
			break;

		case 3:
			nombreShader = "emboss";
			break;

		case 4:
			nombreShader = "voronoi";
			break;

		case 5:
			nombreShader = "GaussianBlur";
			break;

	}

}


//Carga una imagen para utilizarla como fondo
//void AVGMotorGrafico::loadImage(ofxCvColorImage *img, const char* archivo)
//{
//
//	ofImage fileImage;
//	
//	fileImage.loadImage(archivo);
//	fileImage.resize(width, height);
//
//	*img = fileImage.getPixels();
//
//}


//  Establece la textura que se utilizará como entrada de audio
void AVGMotorGrafico::setAudioTexture( ofTexture tex )
{

	texAudio = &tex;

}

//void AVGMotorGrafico::setMaskTexture( ofxCvGrayscaleImage img )
//{
//	maskType = MASK_TYPE_IMG;
//	texMask.loadData(img.getPixelsRef());
//}
//
//void AVGMotorGrafico::setMaskTexture( ofTexture *tex )
//{
//	maskType = MASK_TYPE_TEX;
//	texMaskExterna = tex;
//}


void AVGMotorGrafico::drawFilterGui()
{
	
	effectsManager.drawGui();

}