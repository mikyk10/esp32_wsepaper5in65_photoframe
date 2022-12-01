/**
ESP32_WSEPaper-5in65_PhotoFrame
Copyright (C) 2022 Mitsutaka Kato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package main

import (
	"bytes"
	"fmt"
	"image"
	"io"
	"log"
	"math/rand"
	"net"
	"net/http"
	"os"
	"path"
	"path/filepath"
	"strings"
	"time"

	"github.com/anthonynsimon/bild/adjust"
	"github.com/anthonynsimon/bild/transform"
	"github.com/labstack/echo/v4"
	"github.com/makeworld-the-better-one/dither"
	"github.com/sunshineplan/imgconv"

	"image/color"
	_ "image/gif"
	_ "image/jpeg"
	_ "image/png"
)

const port = 9002

// Palette for Color reduction and Dithering
// palette correspondance is defined here:
// https://www.waveshare.com/wiki/5.65inch_e-Paper_Module_(F)_Manual#Pixel_.26_Byte
var epaper_color_black = color.RGBA{0x2B, 0x2D, 0x3E, 0xff}  // black(0x0)
var epaper_color_white = color.RGBA{0xE2, 0xE0, 0xE1, 0xff}  // white(0x1)
var epaper_color_green = color.RGBA{0x54, 0x6C, 0x57, 0xff}  // green(0x2)
var epaper_color_blue = color.RGBA{0x3A, 0x45, 0x69, 0xff}   // blue(0x3)
var epaper_color_red = color.RGBA{0x9C, 0x57, 0x4D, 0xff}    // red(0x4)
var epaper_color_yellow = color.RGBA{0xDB, 0xD0, 0x71, 0xff} // yellow(0x5)
var epaper_color_orange = color.RGBA{0xB6, 0x6A, 0x58, 0xff} // orange(0x6)

var palette = []color.Color{
	epaper_color_black,
	epaper_color_white,
	epaper_color_green,
	epaper_color_blue,
	epaper_color_red,
	epaper_color_yellow,
	epaper_color_orange,
}

// Device's orientation
type ImageOrientation string

const (
	OrientationNotSpecified = ImageOrientation("")
	Portrait                = ImageOrientation("p")
	Landscape               = ImageOrientation("l")
)

func main() {

	// Initialize echo app
	e := echo.New()

	//
	e.GET("/", func(c echo.Context) error {
		return c.Redirect(http.StatusMovedPermanently, "/random.png")
	})

	// Pick a rondom picture out of some image collection.
	// Following code uses a local storage.
	e.GET("/random:suffix", func(c echo.Context) error {

		// set random seed
		setRandomSeed()

		index := time.Now().YearDay()

		requestedOrientation := ImageOrientation(c.QueryParam("ori"))

		// lists all images under a certain path, randomize its order
		cwd := os.Getenv("PWD")
		dir := path.Join(cwd, "imgs")
		entries := []string{}
		filepath.Walk(dir,
			func(pa string, info os.FileInfo, err error) error {
				if err != nil {
					return err
				}

				if info.IsDir() {
					return nil
				}

				lfName := strings.ToLower(info.Name())
				if strings.HasSuffix(lfName, ".jpg") || strings.HasSuffix(lfName, ".jpeg") || strings.HasSuffix(lfName, ".png") {
					entries = append(entries, pa)
				}

				return nil
			})

		rand.Shuffle(len(entries), func(i, j int) { entries[i], entries[j] = entries[j], entries[i] })

		var filename string

		requestedOrientation = Landscape
		// to be sure the picked filename has image an extension
		for {
			//TODO: cleanup
			filename = entries[index%len(entries)]

			// orientation filter
			if requestedOrientation != OrientationNotSpecified {
				f, _ := os.Open(filename)
				imge, _, _ := image.Decode(f)
				bounds := imge.Bounds()
				f.Close()

				if bounds.Max.X < bounds.Max.Y {
					if requestedOrientation == Portrait {
						break
					}
				} else if requestedOrientation == Landscape {
					break
				}
			} else {
				break
			}

			index++
		}

		var isBinaryRequested bool
		var imfmt imgconv.Format
		var mime string

		// determine the target file conversion type
		suffix := c.Param("suffix")
		switch strings.ToLower(suffix) {
		case ".jpg":
		case ".jpeg":
			imfmt = imgconv.JPEG
			mime = "image/jpeg"
		case ".png":
			imfmt = imgconv.PNG
			mime = "image/png"
		case ".gif":
			imfmt = imgconv.GIF
			mime = "image/gif"
		case ".bin":
			mime = "application/octet-stream"
			isBinaryRequested = true
		default:
			c.String(http.StatusNotAcceptable, "not acceptable")
		}

		// image pre-process
		f, _ := os.Open(filename)
		defer f.Close()
		img := convert(f)

		var buf *bytes.Buffer

		// image conversion into desired type
		if isBinaryRequested {
			buf = encode(img)
			c.Response().Header().Set(echo.HeaderContentLength, fmt.Sprintf("%d", 600*448/2))
		} else {
			buf = &bytes.Buffer{}
			err := imgconv.Write(buf, img, &imgconv.FormatOption{Format: imfmt})
			if err != nil {
				log.Fatalf("failed to write image: %v", err)
			}
		}

		c.Response().Header().Set("X-Sleep-Seconds", "300")

		return c.Stream(http.StatusOK, mime, buf)
	})

	e.Logger.Fatal(e.Start(fmt.Sprintf(":%d", port)))
}

// Encode an image file into the vendor specific binary data
func encode(img image.Image) *bytes.Buffer {
	bounds := img.Bounds()

	buf := bytes.Buffer{}

	var odd uint8 = 1
	var twopx uint8

	// Read 2 pixels, convert the first into MSB 4bits, other goes 4 bits of LSB.
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			c := img.At(x, y)

			for i, px := range palette {
				r1, g1, b1, _ := c.RGBA()
				r2, g2, b2, _ := px.RGBA()

				if r1 == r2 && g1 == g2 && b1 == b2 {
					//fmt.Printf("%+v %d\n", c, i)
					twopx |= uint8(i) << (uint8(4) * odd)
					if odd == 0 {
						//fmt.Printf("%08b\n", twopx)
						buf.WriteByte(twopx)
						twopx = 0
					}
					break
				}
			}

			odd ^= 1
		}
	}

	return &buf
}

// Pre-process an image; gamma correction, cropping to matech up aspect-ratio, resizing, and color reduction, dithering.
// The final data must be 600x448 of 7 colored data.
func convert(r io.Reader) image.Image {
	srcImg, err := imgconv.Decode(r)
	if err != nil {
		log.Fatalf("failed to open image: %v", err)
	}

	// Set Gamma value
	//TODO: automatic gamma correction by image histgram.
	gammaCorrected := adjust.Gamma(srcImg, 1.2)

	//hist := histogram.NewRGBAHistogram(gammaCorrected).Cumulative()

	// the ePaper hardware's aspect ratio
	const asX float64 = 75.0
	const asY float64 = 56.0

	// For automated cropping, rotate the input image -90 degrees, making it landscape.
	rotated := gammaCorrected
	if gammaCorrected.Bounds().Max.X < gammaCorrected.Bounds().Max.Y {
		rotated = transform.Rotate(gammaCorrected, -90.0, &transform.RotationOptions{ResizeBounds: true})
	}

	// image cropping
	bounds := rotated.Bounds()
	calculatedX1 := float64(bounds.Max.Y) * (asX / asY)
	calculatedY1 := float64(bounds.Max.X) * (asY / asX)

	if calculatedX1 > float64(bounds.Max.X) {
		calculatedX1 = float64(bounds.Max.X)
	}

	if calculatedY1 > float64(bounds.Max.Y) {
		calculatedY1 = float64(bounds.Max.Y)
	}
	fmt.Printf("%d, %d\n", bounds.Max.X, bounds.Max.Y)
	fmt.Printf("%f, %f\n", calculatedX1, calculatedY1)

	// number of pixels to shorten
	diffX := float64(bounds.Max.X) - calculatedX1
	diffY := float64(bounds.Max.Y) - calculatedY1
	fmt.Printf("%f, %f\n", diffX, diffY)

	// crop remainder should be equally distributed to left and right edges; calculate the image center point
	cropped := transform.Crop(rotated, image.Rect(int(diffX/2), int(diffY/2), int(calculatedX1+(diffX/2)), int(calculatedY1+(diffY/2))))
	// resize the image into exactly the display module's specification after crop
	resized := imgconv.Resize(cropped, &imgconv.ResizeOption{Width: 600, Height: 448})

	// Color reduction and dithering
	d := dither.NewDitherer(palette)
	d.Matrix = dither.FloydSteinberg
	d.Dither(resized)

	return resized
}

// Gets a HW address for random seed
func setRandomSeed() {
	// Get any Network interface for a MAC address
	ifas, err := net.Interfaces()
	if err != nil {
		panic(err)
	}

	if len(ifas) == 0 {
		panic("Network unavailable")
	}

	ifa := ifas[0]
	var seed int64
	for i := 0; i < len(ifa.HardwareAddr); i++ {
		seed = seed << 8
		seed += int64(ifa.HardwareAddr[i])
	}

	// Fix random seed
	rand.Seed(int64(time.Now().Year()) + seed)
	rand.Seed(time.Now().UnixNano())
}
