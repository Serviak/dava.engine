package com.dava.framework;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import android.content.Context;
import android.graphics.PixelFormat;
import android.hardware.input.InputManager.InputDeviceListener;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.InputDevice;
import android.view.InputDevice.MotionRange;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.ViewGroup.LayoutParams;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import com.bda.controller.ControllerListener;
import com.bda.controller.StateEvent;

public class JNIGLSurfaceView extends GLSurfaceView
{
	private JNIRenderer mRenderer = null;

	private native void nativeOnInput(int action, int id, float x, float y, double time, int source, int tapCount);
	private native void nativeOnKeyDown(int keyCode);
	private native void nativeOnKeyUp(int keyCode);
	private native void nativeOnGamepadElement(int elementKey, float value);
	private native void nativeOnGamepadConnected(int deviceId);
	private native void nativeOnGamepadDisconnected(int deviceId);
	private native void nativeOnGamepadTriggersDisabled();
	
	MOGAListener mogaListener = null;
	GamepadListener gemapadListener = null;

	Integer[] gamepadAxises = null;
	
	boolean[] pressedKeys = new boolean[KeyEvent.getMaxKeyCode() + 1];

	public int lastDoubleActionIdx = -1;
	
	class DoubleTapListener extends GestureDetector.SimpleOnGestureListener{
		JNIGLSurfaceView view;
		
		DoubleTapListener(JNIGLSurfaceView view) {
			this.view = view;
		}
		
		@Override
		public boolean onDoubleTap(MotionEvent e) {
			lastDoubleActionIdx = e.getActionIndex();
			
			view.queueEvent(new InputRunnable(e, 2));
			return true;
		}
	}
	
	GestureDetector doubleTapDetector = null;

	public JNIGLSurfaceView(Context context) 
	{
		super(context);
		Init();
	}

	public JNIGLSurfaceView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		Init();
	}

	private void Init()
	{
		this.getHolder().setFormat(PixelFormat.TRANSLUCENT);

		//setPreserveEGLContextOnPause(true);
		setEGLContextFactory(new JNIContextFactory());
		setEGLConfigChooser(new JNIConfigChooser());

		mRenderer = new JNIRenderer();
		setRenderer(mRenderer);
		setRenderMode(RENDERMODE_CONTINUOUSLY);
		
		mogaListener = new MOGAListener(this);
		gemapadListener = new GamepadListener();
		
		final List<Integer> suppertedAxises = Arrays.asList(
				MotionEvent.AXIS_X,
				MotionEvent.AXIS_Y,
				MotionEvent.AXIS_Z,
				MotionEvent.AXIS_RX,
				MotionEvent.AXIS_RY,
				MotionEvent.AXIS_RZ,
				MotionEvent.AXIS_LTRIGGER,
				MotionEvent.AXIS_RTRIGGER
		);
				
		int[] inputDevices = InputDevice.getDeviceIds();
		Set<Integer> avalibleAxises = new HashSet<Integer>(); 
		for(int id : inputDevices)
		{
			if((InputDevice.getDevice(id).getSources() & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
			{
				nativeOnGamepadConnected(id);
				
				List<MotionRange> ranges = InputDevice.getDevice(id).getMotionRanges();
				for(MotionRange r : ranges)
				{
					int axisId = r.getAxis();
					if(suppertedAxises.contains(axisId))
						avalibleAxises.add(r.getAxis());
				}
			}
		}
		
		gamepadAxises = avalibleAxises.toArray(new Integer[0]);
		
		if(!avalibleAxises.contains(MotionEvent.AXIS_LTRIGGER) || !avalibleAxises.contains(MotionEvent.AXIS_RTRIGGER))
			nativeOnGamepadTriggersDisabled();
		
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
		{
			setPreserveEGLContextOnPause(true);
		}
		
		setDebugFlags(0);
		
		doubleTapDetector = new GestureDetector(JNIActivity.GetActivity(), new DoubleTapListener(this));
	}
	
    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        // Fix lag when text field lost focus, but keyboard not closed yet. 
        outAttrs.imeOptions = JNITextField.GetLastKeyboardIMEOptions();
        outAttrs.inputType = JNITextField.GetLastKeyboardInputType();
        return super.onCreateInputConnection(outAttrs);
    }
    
	@Override
	protected void onSizeChanged(int w, int h, int oldw, int oldh) {
		//YZ rewrite size parameter from fill parent to fixed size
		LayoutParams params = getLayoutParams();
		params.height = h;
		params.width = w;
		super.onSizeChanged(w, h, oldw, oldh);
	}
	
	@Override
	public void onPause()
	{
		super.onPause();
		setRenderMode(RENDERMODE_WHEN_DIRTY);
		mRenderer.OnPause();
	}
	
	@Override
	public void onResume()
	{
		super.onResume();
		setRenderMode(RENDERMODE_CONTINUOUSLY);
	};

	Map<Integer, Integer> tIdMap = new HashMap<Integer, Integer>();
	int nexttId = 1;
	
	class InputRunnable implements Runnable
	{
		class InputEvent
		{
			int id;
			float x;
			float y;
			int source;
			int tapCount;
			
			InputEvent(int id, float x, float y, int source)
			{
				this.id = id;
				this.x = x;
				this.y = y;
				this.source = source;
				this.tapCount = 1;
			}
			
			InputEvent(int id, float x, float y, int source, int tapCount)
			{
				this.id = id;
				this.x = x;
				this.y = y;
				this.source = source;
				this.tapCount = tapCount;
			}
		}

		ArrayList<InputEvent> events;
		double time;
		int action;

		public InputRunnable(final android.view.MotionEvent event, final int tapCount)
		{
			events = new ArrayList<InputEvent>();
			action = event.getActionMasked();
			final int historySize = event.getHistorySize();
			final int eventSource = event.getSource();
			if(action == MotionEvent.ACTION_MOVE)
			{
				final int pointerCount = event.getPointerCount();
				for (int i = 0; i < pointerCount; ++i)
				{
					final int pointerId = event.getPointerId(i);

					if((eventSource & InputDevice.SOURCE_CLASS_POINTER) > 0)
					{
						for (int h = 0; h < historySize; ++h) {
							events.add(new InputEvent(pointerId, event.getHistoricalX(i, h), event.getHistoricalY(i, h), eventSource, tapCount));
						}
						
						events.add(new InputEvent(pointerId, event.getX(i), event.getY(i), eventSource, tapCount));
					}
					if((eventSource & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
					{
						for (int h = 0; h < historySize; ++h) {
							for (int a = 0; a < gamepadAxises.length; ++a) {
								events.add(new InputEvent(gamepadAxises[a], event.getHistoricalAxisValue(gamepadAxises[a], i, h), 0, eventSource, tapCount));
							}
						}
						
						//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
						for (int a = 0; a < gamepadAxises.length; ++a) {
							events.add(new InputEvent(gamepadAxises[a], event.getAxisValue(gamepadAxises[a], i), 0, eventSource, tapCount));
						}
	    			}
	    		}
    		}
    		else
    		{
    			int actionIdx = event.getActionIndex();
    			assert(actionIdx <= event.getPointerCount());
    			
    			final int pointerId = event.getPointerId(actionIdx);
    			for (int h = 0; h < historySize; ++h) {
					events.add(new InputEvent(pointerId, event.getHistoricalX(actionIdx, h), event.getHistoricalY(actionIdx, h), eventSource, tapCount));
				}
    			
    			events.add(new InputEvent(pointerId, event.getX(actionIdx), event.getY(actionIdx), eventSource, tapCount));
    		}
    	}
    	public InputRunnable(final com.bda.controller.MotionEvent event)
    	{
    		action = MotionEvent.ACTION_MOVE;
    		events = new ArrayList<InputEvent>();
        	int pointerCount = event.getPointerCount();
	    	for (int i = 0; i < pointerCount; ++i)
	    	{
	    		//InputEvent::id corresponds to axis id from UIEvent::eJoystickAxisID
	        	events.add(new InputEvent(0, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_X, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(1, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Y, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(2, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_Z, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(5, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RZ, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(6, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_LTRIGGER, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
	        	events.add(new InputEvent(7, event.getAxisValue(com.bda.controller.MotionEvent.AXIS_RTRIGGER, i), 0, InputDevice.SOURCE_CLASS_JOYSTICK));
    		}
    	}
    	
    	int GetTId(int id) {
    		if (tIdMap.containsKey(id))
    			return tIdMap.get(id);
    		
    		int tId = nexttId++;
    		tIdMap.put(id, tId);
    		return tId;
    	}
    	
    	void RemoveTId(int id) {
    		tIdMap.remove(id);
    	}

		@Override
		public void run() {
			for (int i = 0; i < events.size(); ++i) {
				InputEvent event = events.get(i);
				
				if ((event.source & InputDevice.SOURCE_CLASS_JOYSTICK) > 0)
				{
					if(event.id == MotionEvent.AXIS_Y || event.id == MotionEvent.AXIS_RZ || event.id == MotionEvent.AXIS_RY) 
					{
						nativeOnGamepadElement(event.id, -event.x);
					} 
					else 
					{
						nativeOnGamepadElement(event.id, event.x);
					}
				}
				else 
				{
					nativeOnInput(action, GetTId(event.id), event.x, event.y, time, event.source, event.tapCount);
					
					if (action == MotionEvent.ACTION_CANCEL ||
						action == MotionEvent.ACTION_UP ||
						action == MotionEvent.ACTION_POINTER_1_UP ||
						action == MotionEvent.ACTION_POINTER_2_UP ||
						action == MotionEvent.ACTION_POINTER_3_UP) {
						RemoveTId(event.id);
					}
				}
			}
		}
    }

    class KeyInputRunnable implements Runnable {
    	int keyCode;
    	public KeyInputRunnable(int keyCode) {
    		this.keyCode = keyCode;
    	}
    	
    	@Override
    	public void run() {
    		if(KeyEvent.isGamepadButton(keyCode))
    		{
    			nativeOnGamepadElement(keyCode, 1.f);
    		}
    		else
    		{
    			nativeOnKeyDown(keyCode);
    		}
    	}
    }
    
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	// Check keyCode value for pressedKeys array limit
    	if(keyCode >= pressedKeys.length)
    	{
    		return super.onKeyDown(keyCode, event);
    	}
    	
    	if(pressedKeys[keyCode] == false)
    		queueEvent(new KeyInputRunnable(keyCode));
    	pressedKeys[keyCode] = true;
    	
    	if (event.isSystem())
    		return super.onKeyDown(keyCode, event);
    	else
    		return true;
    }
    
    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
    	// Check keyCode value for pressedKeys array limit
    	if(keyCode >= pressedKeys.length)
    	{
    		return super.onKeyUp(keyCode, event);
    	}
    	
    	pressedKeys[keyCode] = false;
    	
    	if(KeyEvent.isGamepadButton(keyCode))
    	{
    		nativeOnGamepadElement(keyCode, 0.f);
    	}
    	else
    	{
    		nativeOnKeyUp(keyCode);
    	}
    	
    	return super.onKeyUp(keyCode, event);
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) 
    {
        boolean isDoubleTap = doubleTapDetector.onTouchEvent(event);
        if (lastDoubleActionIdx >= 0 &&
        	lastDoubleActionIdx == event.getActionIndex() &&
        	event.getAction() == MotionEvent.ACTION_UP) {
        	lastDoubleActionIdx = -1;
        	queueEvent(new InputRunnable(event, 2));
        	isDoubleTap = true;
        }
        if (!isDoubleTap)
            queueEvent(new InputRunnable(event, 1));
        return true;
    }
    
    @Override
    public boolean onGenericMotionEvent(MotionEvent event)
    {
    	queueEvent(new InputRunnable(event, 1));
    	return true;
    }
    
    class GamepadListener implements InputDeviceListener
    {
		@Override
		public void onInputDeviceAdded(int deviceId) 
		{
			nativeOnGamepadConnected(deviceId);
		}

		@Override
		public void onInputDeviceChanged(int deviceId) 
		{	
		}

		@Override
		public void onInputDeviceRemoved(int deviceId) 
		{
			nativeOnGamepadDisconnected(deviceId);
		}
    	
    }
    
    class MOGAListener implements ControllerListener
    {
    	GLSurfaceView parent = null;
    	
    	MOGAListener(GLSurfaceView parent)
    	{
    		this.parent = parent;
    	}
    	
		@Override
		public void onKeyEvent(com.bda.controller.KeyEvent event)
		{
			int keyCode = event.getKeyCode();
			if(event.getAction() == com.bda.controller.KeyEvent.ACTION_DOWN)
			{
		    	if(pressedKeys[keyCode] == false)
		    		parent.queueEvent(new KeyInputRunnable(keyCode));
		    	pressedKeys[keyCode] = true;
			}
			else if(event.getAction() == com.bda.controller.KeyEvent.ACTION_UP)
			{
		    	pressedKeys[keyCode] = false;
		        nativeOnGamepadElement(keyCode, 0.f);
			}
		}
		@Override
		public void onMotionEvent(com.bda.controller.MotionEvent event)
		{
			parent.queueEvent(new InputRunnable(event));
		}
		@Override
		public void onStateEvent(StateEvent event)
		{
			
		}
    }
}
