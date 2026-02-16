#include "Texture.h"

Texture::Texture(string fileName, bool useFrameBuffer) {

	glGenTextures(1, &ID);

	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



	if (useFrameBuffer) {

		//unsigned int FBO;
		glGenFramebuffers(1, &fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, fboID);//!mistake forgot to put fboID instead of ID (black object)

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);


	
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ID, 0);
		
		//render buffer object is responsible for storing depth information after the frame buffer content is used as atexture
		unsigned int RBO;
		glGenRenderbuffers(1, &RBO);
		glBindRenderbuffer(GL_RENDERBUFFER, RBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 1024);
	//	glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

			cout << "ERROR :: FRAMEBUFFER :: Framebuffer is not complete !" << endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
	else {


		stbi_set_flip_vertically_on_load(true);//images usualy have 0,0 at top left but opengl expects that coordinate at bottom left as for texture coordinates
		image = stbi_load((fileName).c_str(), &width, &height, &numColorChannels, 0);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);

	}


}

void Texture::ActivateTexUnit(int i) {

	if (i == 0) {
	
		glActiveTexture(GL_TEXTURE0);
	
	}

	if (i == 1) {

		glActiveTexture(GL_TEXTURE1);

	}

	if (i == 2) {

		glActiveTexture(GL_TEXTURE2);

	}
}

void Texture::Bind() {

	glBindTexture(GL_TEXTURE_2D, ID);

}

void Texture::Unbind() {

	glBindTexture(GL_TEXTURE_2D, 0);

}

void Texture::Delete() {

	glDeleteTextures(1, &ID);

}