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
 * SECTION:element-qreader
 *
 * FIXME:Describe qreader here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! qreader ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstqreader.h"

GST_DEBUG_CATEGORY_STATIC (gst_qreader_debug);
#define GST_CAT_DEFAULT gst_qreader_debug

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
    format = {GRAY8}, \
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

#define gst_qreader_parent_class parent_class
G_DEFINE_TYPE (Gstqreader, gst_qreader, GST_TYPE_ELEMENT);

static void gst_qreader_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_qreader_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_qreader_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_qreader_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);
static GstStateChangeReturn gst_qreader_change_state (GstElement *element, GstStateChange transition);

/* GObject vmethod implementations */

/* initialize the qreader's class */
static void
gst_qreader_class_init (GstqreaderClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_qreader_set_property;
  gobject_class->get_property = gst_qreader_get_property;
  
  gstelement_class->change_state = gst_qreader_change_state;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?", FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
                                       "qreader",
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
gst_qreader_init (Gstqreader * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_qreader_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_qreader_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = FALSE;
}

static void
gst_qreader_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstqreader *filter = GST_QREADER (object);

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
gst_qreader_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstqreader *filter = GST_QREADER (object);

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
gst_qreader_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  Gstqreader *filter;
  gboolean ret;

  filter = GST_QREADER (parent);

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

/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/
/* Functions below print the Capabilities in a human-friendly format */  
static gboolean print_field (GQuark field, const GValue * value, gpointer pfx) {  
  gchar *str = gst_value_serialize (value);  
    
  g_print ("%s  %15s: %s\n", (gchar *) pfx, g_quark_to_string (field), str);  
  g_free (str);  
  return TRUE;  
}  

static void print_caps (const GstCaps * caps, const gchar * pfx) {  
  guint i;  
    
  g_return_if_fail (caps != NULL);  
    
  if (gst_caps_is_any (caps)) {  
    g_print ("%sANY\n", pfx);  
    return;  
  }  
  if (gst_caps_is_empty (caps)) {  
    g_print ("%sEMPTY\n", pfx);  
    return;  
  }  
    
  for (i = 0; i < gst_caps_get_size (caps); i++) {  
    GstStructure *structure = gst_caps_get_structure (caps, i);  
      
    g_print ("%s%s\n", pfx, gst_structure_get_name (structure));  
    gst_structure_foreach (structure, print_field, (gpointer) pfx);  
  }  
}  

/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/
#define CURSOR_SIZE 10
void imgPlotCursor(int nX, int nY, int nWidth, int nHeight, unsigned char *const pGray)
{
	for(int i=nX-CURSOR_SIZE/2; i < nX+CURSOR_SIZE/2; i++)
	{
		if((i<0)||(i>nWidth))
		{
			continue;
		}
		
		pGray[nY*nWidth+i] = 255;
	}
	
	for(int j=nY-CURSOR_SIZE/2; j < nY+CURSOR_SIZE/2; j++)
	{
		if((j<0)||(j>nHeight))
		{
			continue;
		}
		
		pGray[j*nWidth+nX] = 255;
	}
}


/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_qreader_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  Gstqreader *lptrv_QReader = NULL;

  lptrv_QReader = GST_QREADER (parent);

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
  
  print_caps (lptrv_Caps, "      ");
  gst_caps_unref(lptrv_Caps);
  
  
  GstMapInfo ltruv_BufMap = {0};
  int ls32v_Ret = gst_buffer_map (buf, &ltruv_BufMap, GST_MAP_WRITE); 
  //memset (ltruv_BufMap.data, 0, ltruv_BufMap.size/2);
  
  /* Decode */
  zbar_image_t *lptrv_Image = zbar_image_create(); //Create Image
  zbar_image_set_format(lptrv_Image, *(int*)"Y800");
  zbar_image_set_size(lptrv_Image, ls32v_Width, ls32v_Height);
  zbar_image_set_data(lptrv_Image, 
                      ltruv_BufMap.data, 
                      ls32v_Width*ls32v_Height, 
                      NULL);
  
  
  ls32v_Ret = zbar_scan_image(lptrv_QReader->pScanner, lptrv_Image); //Decode 
  if(ls32v_Ret < 0)
  {
  	g_print("%s(): scan failed!\r\n",__func__);
  	
  	return -1;
  }
  
  zbar_point_int_t *lptrv_QRPoint = NULL;  //Extract Centers
  int ls32v_NumCenters = zbar_image_get_center(lptrv_Image, &lptrv_QRPoint);
  
  const zbar_symbol_t *symbol = zbar_image_first_symbol(lptrv_Image);
  for(; symbol; symbol = zbar_symbol_next(symbol)) 
  {
      /* do something useful with results */
      zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
      const char *data = zbar_symbol_get_data(symbol);
      g_print("decoded %s symbol \"%s\" quality=%d\n", zbar_get_symbol_name(typ), 
                                                        data, 
                                                        zbar_symbol_get_quality(symbol));
  }
  
  g_print("%s(): Release img...\r\n", __func__);
  zbar_image_destroy(lptrv_Image);
  lptrv_Image = NULL;
  
  /*Plot Center */ 
  for(int i=0; i<ls32v_NumCenters; i++)
  {
  	imgPlotCursor(lptrv_QRPoint[i].nX, lptrv_QRPoint[i].nY,ls32v_Width, ls32v_Height, ltruv_BufMap.data);
  }
  
  g_print("%s(): Release center...\r\n", __func__);
  zbar_image_free_center(NULL, lptrv_QRPoint); //free Centers
  
  gst_buffer_unmap(buf, &ltruv_BufMap);
  
  

  /* just push out the incoming buffer without touching it */
  return gst_pad_push (lptrv_QReader->srcpad, buf);
}

/*******************************************************************************
Function: 
Description:
Calls:
Called By:
Table Accessed:
Table Updated:
Input:
Output:
Return:
Others:
*******************************************************************************/
static GstStateChangeReturn gst_qreader_change_state (GstElement *element, GstStateChange transition)
{
	GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
	Gstqreader *lptrv_QReader = GST_QREADER (element);
	switch (transition) 
	{
		case GST_STATE_CHANGE_NULL_TO_READY:
			//if (!gst_qreader_allocate_memory (lptrv_QReader))
			//{
			//	return GST_STATE_CHANGE_FAILURE;
			//}
			
			/* create a reader */
		    lptrv_QReader->pScanner = zbar_image_scanner_create();
		
		    /* configure the reader */
		    zbar_image_scanner_set_config(lptrv_QReader->pScanner, 0, ZBAR_CFG_ENABLE, 0);
		    zbar_image_scanner_set_config(lptrv_QReader->pScanner, ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
		    //zbar_image_scanner_set_config(lptrv_Scanner, ZBAR_ISBN13, ZBAR_CFG_ENABLE, 1);
		    //zbar_image_scanner_set_config(lptrv_Scanner, ZBAR_EAN13, ZBAR_CFG_ENABLE, 1);
		    zbar_image_scanner_set_config(lptrv_QReader->pScanner, 0, ZBAR_CFG_X_DENSITY, 1);
		    zbar_image_scanner_set_config(lptrv_QReader->pScanner, 0, ZBAR_CFG_Y_DENSITY, 1);
			break;
		default:
			break;
	}
	ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		return ret;
	}
	switch (transition) 
	{
		case GST_STATE_CHANGE_READY_TO_NULL:
			zbar_image_scanner_destroy(lptrv_QReader->pScanner);
			lptrv_QReader->pScanner = NULL;
			
			//gst_qreader_free_memory (lptrv_QReader);
			break;
		default:
			break;
	}
	
	return ret;
}
/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
qreader_init (GstPlugin * qreader)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template qreader' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_qreader_debug, "qreader",
      0, "Template qreader");

  return gst_element_register (qreader, "qreader", GST_RANK_NONE,
      GST_TYPE_QREADER);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstqreader"
#endif

/* gstreamer looks for this structure to register qreaders
 *
 * exchange the string 'Template qreader' with your qreader description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    qreader,
    "Template qreader",
    qreader_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
