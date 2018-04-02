/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2018 Wangsh <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-hybridbin
 *
 * FIXME:Describe hybridbin here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! hybridbin ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gsthybridbin.h"

#include "ImageReaderSource.h"
#include <zxing/detector/Detector.h>
#include <zxing/common/HybridBinarizer.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

using namespace std;
using namespace zxing;
//using namespace zxing::multi;
using namespace zxing::qrcode;

GST_DEBUG_CATEGORY_STATIC (gst_hybrid_bin_debug);
#define GST_CAT_DEFAULT gst_hybrid_bin_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    //GST_STATIC_CAPS ("ANY")
    GST_STATIC_CAPS("video/x-raw, \
    format = {RGBx}, \
    width = (int) [1, 3000], \
    height = (int) [1, 4000], \
    framerate = (fraction) [0, 30]")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    //GST_STATIC_CAPS ("ANY")
    GST_STATIC_CAPS("video/x-raw, \
    format = {GRAY8}, \
    width = (int) [1, 3000], \
    height = (int) [1, 4000], \
    framerate = (fraction) [0, 30]")
    );

#define gst_hybrid_bin_parent_class parent_class
G_DEFINE_TYPE (GstHybridBin, gst_hybrid_bin, GST_TYPE_ELEMENT);

static void gst_hybrid_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_hybrid_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_hybrid_bin_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_hybrid_bin_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);

/* GObject vmethod implementations */

/* initialize the hybridbin's class */
static void
gst_hybrid_bin_class_init (GstHybridBinClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_hybrid_bin_set_property;
  gobject_class->get_property = gst_hybrid_bin_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
    "HybridBin",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    "Wangsh <<user@hostname.org>>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_hybrid_bin_init (GstHybridBin * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_hybrid_bin_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_hybrid_bin_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = FALSE;
}

static void
gst_hybrid_bin_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstHybridBin *filter = GST_HYBRIDBIN (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_hybrid_bin_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstHybridBin *filter = GST_HYBRIDBIN (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_hybrid_bin_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  GstHybridBin *filter;
  gboolean ret;

  filter = GST_HYBRIDBIN (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps * caps;

      gst_event_parse_caps (event, &caps);
      /* do something with the caps */

      /* and forward */
      ret = gst_pad_event_default (pad, parent, event);
      break;
    }
    default:
      ret = gst_pad_event_default (pad, parent, event);
      break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_hybrid_bin_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
    
  GstHybridBin *lptrv_QReader = NULL;

  lptrv_QReader = GST_HYBRIDBIN (parent);

  if (lptrv_QReader->silent == FALSE)
  {
	g_print ("Have data of size %" G_GSIZE_FORMAT" bytes!\n", gst_buffer_get_size (buf));
	g_print("PTS=%d, DTS=%d, OS=%d, OS_End=%d\r\n", GST_BUFFER_PTS(buf), GST_BUFFER_DTS(buf), GST_BUFFER_OFFSET(buf), GST_BUFFER_OFFSET_END(buf));
  }
  
  GstCaps *lptrv_Caps = gst_pad_get_current_caps(pad);
  if(lptrv_Caps == NULL)
  {
  	g_print("Get caps error");
  }
  
  gint ls32v_Width = 0;
  gint ls32v_Height = 0;
  GstStructure *lptrv_GstStruct = gst_caps_get_structure(lptrv_Caps, 0);
  
  gst_structure_get_int (lptrv_GstStruct, "width", &ls32v_Width);
  gst_structure_get_int (lptrv_GstStruct, "height", &ls32v_Height);
  g_print("img width=%d, height=%d\r\n", ls32v_Width, ls32v_Height);
  
  //print_caps (lptrv_Caps, "      ");
  gst_caps_unref(lptrv_Caps);
  
  
  GstMapInfo ltruv_BufMap = {0};
  int ls32v_Ret = gst_buffer_map (buf, &ltruv_BufMap, GST_MAP_WRITE); 
  
  /* Binarizer */
  	Ref<LuminanceSource> source;
 
    try {
      //zxing::ArrayRef<char> image = zxing::ArrayRef<char>(pImg, nWidth*nHeight);
      //source = new ImageReaderSource(image, nWidth, nHeight, 1);
      source = ImageReaderSource::create((char *)ltruv_BufMap.data, ls32v_Width, ls32v_Height);
    } catch (const zxing::IllegalArgumentException &e) {
      cerr << e.what() << " (ignoring)" << endl;
      //continue;
    }
    
    Ref<Binarizer> binarizer;
    binarizer = new HybridBinarizer(source); //implements a local thresholding algorithm
    Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));

	Ref<BitMatrix> lclav_BitMatrixRef = binary->getBlackMatrix();
    for(int i=0; i<ls32v_Height; i++)
    {
    	for(int j=0; j< ls32v_Width; j++)
    	{
    		unsigned char lu8v_Gray = 0;
    		lu8v_Gray = lclav_BitMatrixRef->get(j,i) ? 0:255;
    		ltruv_BufMap.data[(i*ls32v_Width+j)*4] = lu8v_Gray;
    		ltruv_BufMap.data[(i*ls32v_Width+j)*4+1] = lu8v_Gray;
    		ltruv_BufMap.data[(i*ls32v_Width+j)*4+2] = lu8v_Gray;
    		ltruv_BufMap.data[(i*ls32v_Width+j)*4+3] = 0;
    	}
    }
  
  gst_buffer_unmap(buf, &ltruv_BufMap);

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (lptrv_QReader->srcpad, buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
hybridbin_init (GstPlugin * hybridbin)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template hybridbin' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_hybrid_bin_debug, "hybridbin",
      0, "Template hybridbin");

  return gst_element_register (hybridbin, "hybridbin", GST_RANK_NONE,
      GST_TYPE_HYBRIDBIN);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirsthybridbin"
#endif

/* gstreamer looks for this structure to register hybridbins
 *
 * exchange the string 'Template hybridbin' with your hybridbin description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    hybridbin,
    "Template hybridbin",
    hybridbin_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
