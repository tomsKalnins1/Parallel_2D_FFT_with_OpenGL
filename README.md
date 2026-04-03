<h1>2D Parallel Fast Fourier Transform</h1>
<p style="text-align: justify;">

This is a naive implementation of the parallel 2D fft.
Takes in an image that has a power of 2 as the hight and width. When run the program displays the separate frequencies being added up in the  

</p>
<img src="visualized_2d_frequecies/image_fft_ifft_image.png" style="float: left; margin-right: 15px;">
<p style="text-align: justify;">

Above one the left is the original input image, in the middle is the an image I used to store the complex outputs of the FFT in to later use for the IFFT result of which is the image on the right.
However the FFT output can be interpreted in terms of amplitude and phase and those value stored in a texture or image as below (phase is image in the middle, amplitude is the rightmost image).

</p>
<img src="visualized_2d_frequecies/FFT_2D_IMG_TO_MAGNITUDE_PHASE.png" style="float: left; margin-right: 15px;">

<h2 style="clear: both">Reading the input image</h2>
<p style="text-align: justify;">
At this point the program takes in black and white RGBA image as a texture. Compute shader performs parallel version of 1D fft on each row of the image, at each dispatch there are as many workgroups as there are rows, while each work group has number of sample 
</p>
<h2 style="clear: both">Wrapped Image Example</h2>

<p style="text-align: justify;">
This text will wrap around the image on the right side. GitHub still supports
the legacy <code>align="left"</code> attribute for images, which is actually
the most consistent way to achieve this effect in a README.

You can keep writing more text here and it will continue flowing around the image,
similar to how word processors handle text wrapping.
This text will wrap around the image on the right side. GitHub still supports
the legacy <code>align="left"</code> attribute for images, which is actually
the most consistent way to achieve this effect in a README.

You can keep writing more text here and it will continue flowing around the image,
similar to how word processors handle text wrapping.
This text will wrap around the image on the right side. GitHub still supports
the legacy <code>align="left"</code> attribute for images, which is actually
the most consistent way to achieve this effect in a README.

You can keep writing more text here and it will continue flowing around the image,
similar to how word processors handle text wrapping.
This text will wrap around the image on the right side. GitHub still supports
the legacy <code>align="left"</code> attribute for images, which is actually
the most consistent way to achieve this effect in a README.

You can keep writing more text here and it will continue flowing around the image,
similar to how word processors handle text wrapping.
This text will wrap around the image on the right side. GitHub still supports
the legacy <code>align="left"</code> attribute for images, which is actually
the most consistent way to achieve this effect in a README.

You can keep writing more text here and it will continue flowing around the image,
similar to how word processors handle text wrapping.
</p>

<p style="text-align: justify;">
Once the text is long enough, it will naturally clear below the image.
</p>
