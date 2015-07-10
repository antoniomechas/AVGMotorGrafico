#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"
#include "ofxPostProcessing.h"
#include "EffectsManager.h"

#define MILIS_CHECK 1000			// Mirará si el shader ha cambiado cada 1000 milis = 1 seg
//#define MASK_TYPE_IMG 0
//#define MASK_TYPE_TEX 1

class AVGMotorGrafico
{
	
	public:
		
		//enum tipoOrigen {ORIGEN_IMAGEN, ORIGEN_CAPTURA_DRAW};

		void		draw						( );
		void		setup						( int w, int h );
		void		update						( ofxCvGrayscaleImage *img );
		void		beginCaptureDraw			( );
		void		endCaptureDraw				( );
		void		setAudioTexture				( ofTexture tex );						// Establece la textura que se utilizará como entrada de audio
		void		drawFilterGui				( );
		//void		setMaskTexture( ofxCvGrayscaleImage img);							// le pasa una imagen parar guardar la textura
		//void		setMaskTexture( ofTexture *tex);									// le pasa una textura externa para que la utilice como mascara
		//void		setImagenOrigen				( tipoOrigen t);

		ofxToggle		bUsaShader;				//  Dibujará utilizando el shader
		ofxToggle		bOn;					//  si está activo o no
		ofxToggle		bClipping;				//  si se hace clipping
		ofxToggle		bInvertir;				//  si invierte la imagen antes de dibujarla
		ofxToggle		bDontDraw;				//  No dibuja, guarda el fbo para que se utilice después
		ofxIntSlider	iShader;				//  shader a utilizar
		ofxIntSlider	iKernelSize;			//  Para el gaussian blur shader
		ofxToggle		bFlash;					//  Establece el modo Flash (parpadeo, cambia entre invertido y no invertido). Independiente del valor de bInvertir
		ofxIntSlider	iIntervaloMin;			//  intervarlo en milisegundos para cambiar entre invertido o no
		ofxIntSlider	iIntervaloMax;			//  intervarlo en milisegundos para cambiar entre invertido o no
		ofxToggle		bUsePingPong;			//  Establece si utiliza el shader con un FBO ping pong, para hacer un blur, guardar el estado y hacer alpha damping
		ofxFloatSlider	fPingPongAlphaDamping;	//  Valor de alpha damping para el shader de ping pong [0...1]	
		ofxToggle		bPostBloom;				//  ofxPostProcessing
		ofxToggle		bPostFxaa;				//  ofxPostProcessing

		ofVec2f			shaderCenter;			//  Centro del objeto del shader
		float			shaderHeight;			//  Alto del objeto
		float			shaderWidth;			//	Ancho del objeto

		//bool			bUsaImgFondo;			//  se utilizará la imagen cargada como fondo

		float			fAlphaShader;			//  Entre 0...1 -> se le suma en el shader al R+G+B, para aumentar el brillo

		CvPoint			ptoClipping[4];			//  define el area de clipping, lo lee desde el fichero de configuración el padre

		ofTexture		*texAudio;				//  se establecerá desde setAudioTexture() para que sirva de input a los shaders que trabajan con sonido
		//ofTexture		texMask;				//  Para los shaders que necesitan mascara
		//ofTexture		*texMaskExterna;		//  Para los shaders que necesitan mascara, se establece con setMaskTexture()
		float			volumen;

		ofFbo			fboOut;					// Se utliza cuando no se quiere dibujar en pantalla, sino en un fbo para reutilizarlo después


	private:
		
		void				renderDummy();
		void				drawClipping();														//  Recorta la imagen
		void				setNombreShader();													//  Rellena la variable nombreShader con el nombre del shader escogido con iShader.
		//void				loadImage(ofxCvColorImage *img, const char* archivo);				//  Carga una imagen desde el disco
		//void				aplicaEfectoSobreFondo();											//  combina la imagen de fondo con la que hay que dibujar para crear efectos sobre la imagen de fondo
		void				drawShader();														//  Dibuja lo que toque con el shader especificado
		void				setupPingPong();													//  Inicializa el shader y los FBOs para el ping pong
		void				drawPingPong( ofFbo *fboSrc);										//  Dibuja sobre el fbo de ping pong para aplicarle el shader de blur y alpha damping
		void				drawInvertir( ofFbo *fboSrc);

		string				nombreShader;

		Poco::Timestamp		lastFragTimestamp, lastVertTimestamp;								//  Para saber si hace falta recargar el archivo de shader

		ofShader			shader1;		
		//ofShader			shader2;
		ofShader			shaderI;
		ofFbo				fbo;
		ofFbo				fbo2;
		ofFbo				fboTex;
		ofFbo				fboNoARB;
		//ofxCvGrayscaleImage	imagenOrigen;
		//ofxCvColorImage		imagenDestino;		//  Se utilizará para dibujar
		//ofxCvColorImage     imagenFondo;		//  Se carga de disco para utilizarla como fondo o para darle efectos
		//ofxCvColorImage     imagenTmp;			//  para calculos intermedios
		//ofxCvColorImage     textura;			//  textura para pasársela al shader

		//ofImage				imgTmp;

		int					width;
		int					height;

		//tipoOrigen			tOrigen;			//  imagen : Dibujará la imagen que se le pase en update() con los procesos especificados
												//  CapturaDraw: Capturará el draw() con begin() y end(), y lo guardará en una textura para utilizarlo en el shader
		
		unsigned long long	lastTimeChecked;	//  guarda la última vez que se miró si el archivo del shader se había actualizado
		unsigned long long	lastTimeInterval;	//  guarda la última vez que se cambió de un estado a otro según el intervalo (de invertido a no)
		
		//int					maskType;			// 0 = mascara es texMask, 1 = mascara es un referencia a una textura externa en texMaskExterna

		ofxPostProcessing	post;

		EffectsManager		effectsManager;

		//Para llevar donde se va escribiendo en cada paso
		ofFbo				*nextFbo;

		// Para el fbo de pingPong
		ofFbo					pingPong[2];
		ofFbo					*src;
		ofFbo					*dst;
		ofShader				shaderPingPong;

};