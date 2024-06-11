
//NOTE: this header redeclares the DComp COM interfaces in a way that's useable from C
// (in particular, it declares inherited methods of parent interfaces, which are needed in C)

//---------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//---------------------------------------------------------------------------
#pragma once

#include <d2dbasetypes.h>   // for D2D_MATRIX_3X2_F
#ifndef D3DMATRIX_DEFINED
#include <d3d9types.h>      // for D3DMATRIX
#endif
#include <d2d1_1.h>         // for D2D1_COMPOSITE_MODE
#include <winapifamily.h>

#include <dcomptypes.h>     // for CompositionSurfaceType
#include <dcompanimation.h> // for IDirectCompositionAnimation interface

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if (NTDDI_VERSION >= NTDDI_WIN8)

typedef interface IDCompositionAffineTransform2DEffect   IDCompositionAffineTransform2DEffect;
typedef interface IDCompositionAnimation                 IDCompositionAnimation;
typedef interface IDCompositionArithmeticCompositeEffect IDCompositionArithmeticCompositeEffect;
typedef interface IDCompositionBlendEffect               IDCompositionBlendEffect;
typedef interface IDCompositionBrightnessEffect          IDCompositionBrightnessEffect;
typedef interface IDCompositionClip                      IDCompositionClip;
typedef interface IDCompositionColorMatrixEffect         IDCompositionColorMatrixEffect;
typedef interface IDCompositionCompositeEffect           IDCompositionCompositeEffect;
typedef interface IDCompositionDevice                    IDCompositionDevice;
typedef interface IDCompositionEffect                    IDCompositionEffect;
typedef interface IDCompositionEffectGroup               IDCompositionEffectGroup;
typedef interface IDCompositionFilterEffect              IDCompositionFilterEffect;
typedef interface IDCompositionGaussianBlurEffect        IDCompositionGaussianBlurEffect;
typedef interface IDCompositionHueRotationEffect         IDCompositionHueRotationEffect;
typedef interface IDCompositionLinearTransferEffect      IDCompositionLinearTransferEffect;
typedef interface IDCompositionMatrixTransform           IDCompositionMatrixTransform;
typedef interface IDCompositionMatrixTransform3D         IDCompositionMatrixTransform3D;
typedef interface IDCompositionRectangleClip             IDCompositionRectangleClip;
typedef interface IDCompositionRotateTransform           IDCompositionRotateTransform;
typedef interface IDCompositionRotateTransform3D         IDCompositionRotateTransform3D;
typedef interface IDCompositionSaturationEffect          IDCompositionSaturationEffect;
typedef interface IDCompositionScaleTransform            IDCompositionScaleTransform;
typedef interface IDCompositionScaleTransform3D          IDCompositionScaleTransform3D;
typedef interface IDCompositionShadowEffect              IDCompositionShadowEffect;
typedef interface IDCompositionSkewTransform             IDCompositionSkewTransform;
typedef interface IDCompositionSurface                   IDCompositionSurface;
typedef interface IDCompositionTableTransferEffect       IDCompositionTableTransferEffect;
typedef interface IDCompositionTarget                    IDCompositionTarget;
typedef interface IDCompositionTransform                 IDCompositionTransform;
typedef interface IDCompositionTransform3D               IDCompositionTransform3D;
typedef interface IDCompositionTranslateTransform        IDCompositionTranslateTransform;
typedef interface IDCompositionTranslateTransform3D      IDCompositionTranslateTransform3D;
typedef interface IDCompositionTurbulenceEffect          IDCompositionTurbulenceEffect;
typedef interface IDCompositionVirtualSurface            IDCompositionVirtualSurface;
typedef interface IDCompositionVisual                    IDCompositionVisual;

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionCreateDevice
//
//  Synopsis:
//      Creates a new DirectComposition device object, which can be used to create
//      other DirectComposition objects.
//
//------------------------------------------------------------------------------
STDAPI DCompositionCreateDevice(
    _In_opt_ IDXGIDevice *dxgiDevice,
    _In_ REFIID iid,
    _Outptr_ void **dcompositionDevice
    );

#if (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)
//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionCreateDevice2
//
//  Synopsis:
//      Creates a new DirectComposition device object, which can be used to create
//      other DirectComposition objects.
//
//------------------------------------------------------------------------------
STDAPI DCompositionCreateDevice2(
    _In_opt_ IUnknown *renderingDevice,
    _In_ REFIID iid,
    _Outptr_ void **dcompositionDevice
    );
#endif  // (_WIN32_WINNT >= _WIN32_WINNT_WINBLUE)

#if (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)
//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionCreateDevice3
//
//  Synopsis:
//      Creates a new DirectComposition device object, which can be used to create
//      other DirectComposition objects.
//
//------------------------------------------------------------------------------
STDAPI DCompositionCreateDevice3(
    _In_opt_ IUnknown *renderingDevice,
    _In_ REFIID iid,
    _Outptr_ void **dcompositionDevice
    );

#endif  // (_WIN32_WINNT >= _WIN32_WINNT_WINTHRESHOLD)

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionCreateSurfaceHandle
//
//  Synopsis:
//      Creates a new composition surface object, which can be bound to a
//      DirectX swap chain or swap buffer or to a GDI bitmap and associated
//      with a visual.
//
//------------------------------------------------------------------------------
STDAPI DCompositionCreateSurfaceHandle(
    _In_ DWORD desiredAccess,
    _In_opt_ SECURITY_ATTRIBUTES *securityAttributes,
    _Out_ HANDLE *surfaceHandle
    );

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionAttachMouseWheelToHwnd
//
//  Synopsis:
//      Creates an Interaction/InputSink to route mouse wheel messages to the
//      given HWND. After calling this API, the device owning the visual must
//      be committed.
//
//------------------------------------------------------------------------------
STDAPI DCompositionAttachMouseWheelToHwnd(
    _In_ IDCompositionVisual* visual,
    _In_ HWND hwnd,
    _In_ BOOL enable
    );

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionAttachMouseDragToHwnd
//
//  Synopsis:
//      Creates an Interaction/InputSink to route mouse button down and any
//      subsequent move and up events to the given HWND. There is no move
//      thresholding; when enabled, all events including and following the down
//      are unconditionally redirected to the specified window. After calling this
//      API, the device owning the visual must be committed.
//
//------------------------------------------------------------------------------
STDAPI DCompositionAttachMouseDragToHwnd(
    _In_ IDCompositionVisual* visual,
    _In_ HWND hwnd,
    _In_ BOOL enable
    );

#if (NTDDI_VERSION >= NTDDI_WIN10_CO)


//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionGetCurrentFrameId
//
//  Synopsis:
//      Returns the frameId of the most recently started composition frame.
//
//------------------------------------------------------------------------------
STDAPI DCompositionGetFrameId(
    _In_ COMPOSITION_FRAME_ID_TYPE frameIdType,
    _Out_ COMPOSITION_FRAME_ID* frameId);

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionGetStatistics
//
//  Synopsis:
//      Returns statistics for the requested frame, as well as an optional list
//      of all target ids that existed at that time.
//
//------------------------------------------------------------------------------
STDAPI DCompositionGetStatistics(
    _In_ COMPOSITION_FRAME_ID frameId,
    _Out_ COMPOSITION_FRAME_STATS* frameStats,
    _In_ UINT targetIdCount,
    _Out_writes_opt_(targetCount) COMPOSITION_TARGET_ID* targetIds,
    _Out_opt_ UINT* actualTargetIdCount);

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionGetCompositorStatistics
//
//  Synopsis:
//      Returns compositor target statistics for the requested frame.
//
//------------------------------------------------------------------------------
STDAPI DCompositionGetTargetStatistics(
    _In_ COMPOSITION_FRAME_ID frameId,
    _In_ const COMPOSITION_TARGET_ID* targetId,
    _Out_ COMPOSITION_TARGET_STATS* targetStats);

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionBoostCompositorClock
//
//  Synopsis:
//      Requests compositor to temporarily increase framerate.
//
//------------------------------------------------------------------------------
STDAPI DCompositionBoostCompositorClock(
    _In_ BOOL enable);

//+-----------------------------------------------------------------------------
//
//  Function:
//      DCompositionWaitForCompositorClock
//
//  Synopsis:
//      Waits for a compositor clock tick, other events, or a timeout.
//
//------------------------------------------------------------------------------
STDAPI_(DWORD) DCompositionWaitForCompositorClock(
    _In_range_(0, DCOMPOSITION_MAX_WAITFORCOMPOSITORCLOCK_OBJECTS) UINT count,
    _In_reads_opt_(count) const HANDLE* handles,
    _In_ DWORD timeoutInMs);

#endif  // (NTDDI_VERSION >= NTDDI_WIN10_CO)

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionDevice
//
//  Synopsis:
//      Serves as the root factory for all other DirectComposition objects and
//      controls transactional composition.
//
//------------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE IDCompositionDevice
DECLARE_INTERFACE_IID_(IDCompositionDevice, IUnknown, "C37EA93A-E7AA-450D-B16F-9746CB0407F3")
{
    STDMETHOD(QueryInterface)(THIS_
        REFIID riid,
        _Outptr_ void** ppObject
        ) PURE;

    STDMETHOD(AddRef)(THIS) PURE;
    STDMETHOD(Release)(THIS) PURE;

    // Commits all DirectComposition commands pending on this device.
    STDMETHOD(Commit)(THIS
        ) PURE;

    // Waits for the last Commit to be processed by the composition engine
    STDMETHOD(WaitForCommitCompletion)(THIS
        ) PURE;

    // Gets timing information about the composition engine.
    STDMETHOD(GetFrameStatistics)(THIS_
        _Out_ DCOMPOSITION_FRAME_STATISTICS *statistics
        ) PURE;

    // Creates a composition target bound to a window represented by an HWND.
    STDMETHOD(CreateTargetForHwnd)(THIS_
        _In_ HWND hwnd,
        BOOL topmost,
        _Outptr_ IDCompositionTarget **target
        ) PURE;

    // Creates a new visual object.
    STDMETHOD(CreateVisual)(THIS_
        _Outptr_ IDCompositionVisual **visual
        ) PURE;

    // Creates a DirectComposition surface object
    STDMETHOD(CreateSurface)(THIS_
        _In_ UINT width,
        _In_ UINT height,
        _In_ DXGI_FORMAT pixelFormat,
        _In_ DXGI_ALPHA_MODE alphaMode,
        _Outptr_ IDCompositionSurface **surface
        ) PURE;

    // Creates a DirectComposition virtual surface object
    STDMETHOD(CreateVirtualSurface)(THIS_
        _In_ UINT initialWidth,
        _In_ UINT initialHeight,
        _In_ DXGI_FORMAT pixelFormat,
        _In_ DXGI_ALPHA_MODE alphaMode,
        _Outptr_ IDCompositionVirtualSurface **virtualSurface
        ) PURE;

    // Creates a surface wrapper around a pre-existing surface that can be associated with one or more visuals for composition.
    STDMETHOD(CreateSurfaceFromHandle)(THIS_
        _In_ HANDLE handle,
        _Outptr_ IUnknown **surface
        ) PURE;

    // Creates a wrapper object that represents the rasterization of a layered window and which can be associated with a visual for composition.
    STDMETHOD(CreateSurfaceFromHwnd)(THIS_
        _In_ HWND hwnd,
        _Outptr_ IUnknown **surface
        ) PURE;

    // Creates a 2D translation transform object.
    STDMETHOD(CreateTranslateTransform)(THIS_
        _Outptr_ IDCompositionTranslateTransform **translateTransform
        ) PURE;

    // Creates a 2D scale transform object.
    STDMETHOD(CreateScaleTransform)(THIS_
        _Outptr_ IDCompositionScaleTransform **scaleTransform
        ) PURE;

    // Creates a 2D rotation transform object.
    STDMETHOD(CreateRotateTransform)(THIS_
        _Outptr_ IDCompositionRotateTransform **rotateTransform
        ) PURE;

    // Creates a 2D skew transform object.
    STDMETHOD(CreateSkewTransform)(THIS_
        _Outptr_ IDCompositionSkewTransform **skewTransform
        ) PURE;

    // Creates a 2D 3x2 matrix transform object.
    STDMETHOD(CreateMatrixTransform)(THIS_
        _Outptr_ IDCompositionMatrixTransform **matrixTransform
        ) PURE;

    // Creates a 2D transform object that holds an array of 2D transform objects.
    STDMETHOD(CreateTransformGroup)(THIS_
        _In_reads_(elements) IDCompositionTransform **transforms,
        UINT elements,
        _Outptr_ IDCompositionTransform **transformGroup
        ) PURE;

    // Creates a 3D translation transform object.
    STDMETHOD(CreateTranslateTransform3D)(THIS_
        _Outptr_ IDCompositionTranslateTransform3D **translateTransform3D
        ) PURE;

    // Creates a 3D scale transform object.
    STDMETHOD(CreateScaleTransform3D)(THIS_
        _Outptr_ IDCompositionScaleTransform3D **scaleTransform3D
        ) PURE;

    // Creates a 3D rotation transform object.
    STDMETHOD(CreateRotateTransform3D)(THIS_
        _Outptr_ IDCompositionRotateTransform3D **rotateTransform3D
        ) PURE;

    // Creates a 3D 4x4 matrix transform object.
    STDMETHOD(CreateMatrixTransform3D)(THIS_
        _Outptr_ IDCompositionMatrixTransform3D **matrixTransform3D
        ) PURE;

    // Creates a 3D transform object that holds an array of 3D transform objects.
    STDMETHOD(CreateTransform3DGroup)(THIS_
        _In_reads_(elements) IDCompositionTransform3D **transforms3D,
        UINT elements,
        _Outptr_ IDCompositionTransform3D **transform3DGroup
        ) PURE;

    // Creates an effect group
    STDMETHOD(CreateEffectGroup)(THIS_
        _Outptr_ IDCompositionEffectGroup **effectGroup
        ) PURE;

    // Creates a clip object that can be used to clip the contents of a visual subtree.
    STDMETHOD(CreateRectangleClip)(THIS_
        _Outptr_ IDCompositionRectangleClip **clip
        ) PURE;

    // Creates an animation object
    STDMETHOD(CreateAnimation)(THIS_
        _Outptr_ IDCompositionAnimation **animation
        ) PURE;

    // Returns the states of the app's DX device and DWM's dx devices
    STDMETHOD(CheckDeviceState)(THIS_
        _Out_ BOOL *pfValid
        ) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionTarget
//
//  Synopsis:
//      An IDCompositionTarget interface represents a binding between a
//      DirectComposition visual tree and a destination on top of which the
//      visual tree should be composed.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionTarget
DECLARE_INTERFACE_IID_(IDCompositionTarget, IUnknown, "eacdd04c-117e-4e17-88f4-d1b12b0e3d89")
{
    STDMETHOD(QueryInterface)(THIS_
        REFIID riid,
        _Outptr_ void** ppObject
        ) PURE;

    STDMETHOD(AddRef)(THIS) PURE;
    STDMETHOD(Release)(THIS) PURE;

    // Sets the root visual
    STDMETHOD(SetRoot)(THIS_
        _In_opt_ IDCompositionVisual* visual
        ) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionVisual
//
//  Synopsis:
//      An IDCompositionVisual interface represents a visual that participates in
//      a visual tree.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionVisual
DECLARE_INTERFACE_IID_(IDCompositionVisual, IUnknown, "4d93059d-097b-4651-9a60-f0f25116e2f3")
{
    STDMETHOD(QueryInterface)(THIS_
        REFIID riid,
        _Outptr_ void** ppObject
        ) PURE;

    STDMETHOD(AddRef)(THIS) PURE;
    STDMETHOD(Release)(THIS) PURE;

    // Changes the value of OffsetX property
    STDMETHOD(SetOffsetX)(THIS_
        float offsetX
        ) PURE;

    // Animates the value of the OffsetX property.
    STDMETHOD(SetOffsetXAnimation)(THIS_
        _In_ IDCompositionAnimation* animation
        ) PURE;

    // Changes the value of OffsetY property
    STDMETHOD(SetOffsetY)(THIS_
        float offsetY
        ) PURE;

    // Animates the value of the OffsetY property.
    STDMETHOD(SetOffsetYAnimation)(THIS_
        _In_ IDCompositionAnimation* animation
        ) PURE;

    // Sets the matrix that modifies the coordinate system of this visual.
    STDMETHOD(SetTransformMatrixRef)(THIS_
        const D2D_MATRIX_3X2_F* matrix
        ) PURE;

    // Sets the transformation object that modifies the coordinate system of this visual.
    STDMETHOD(SetTransform)(THIS_
        _In_opt_ IDCompositionTransform* transform
        ) PURE;

    // Sets the visual that should act as this visual's parent for the
    // purpose of establishing a base coordinate system.
    STDMETHOD(SetTransformParent)(THIS_
        _In_opt_ IDCompositionVisual *visual
        ) PURE;

    // Sets the effect object that is applied during the rendering of this visual
    STDMETHOD(SetEffect)(THIS_
        _In_opt_ IDCompositionEffect *effect
        ) PURE;

    // Sets the mode to use when interpolating pixels from bitmaps drawn not
    // exactly at scale and axis-aligned.
    STDMETHOD(SetBitmapInterpolationMode)(THIS_
        _In_ enum DCOMPOSITION_BITMAP_INTERPOLATION_MODE interpolationMode
        ) PURE;

    // Sets the mode to use when drawing the edge of bitmaps that are not
    // exactly axis-aligned and at precise pixel boundaries.
    STDMETHOD(SetBorderMode)(THIS_
        _In_ enum DCOMPOSITION_BORDER_MODE borderMode
        ) PURE;

    // Sets the clip object that restricts the rendering of this visual to a D2D rectangle.
    STDMETHOD(SetClipRect)(THIS_
        const D2D_RECT_F* rect
        ) PURE;

    // Sets the clip object that restricts the rendering of this visual to a rectangle.
    STDMETHOD(SetClip)(THIS_
        _In_opt_ IDCompositionClip* clip
        ) PURE;

    // Associates a bitmap with a visual
    STDMETHOD(SetContent)(THIS_
        _In_opt_ IUnknown *content
        ) PURE;

    // Adds a visual to the children list of another visual.
    STDMETHOD(AddVisual)(THIS_
        _In_ IDCompositionVisual* visual,
        BOOL insertAbove,
        _In_opt_ IDCompositionVisual* referenceVisual
        ) PURE;

    // Removes a visual from the children list of another visual.
    STDMETHOD(RemoveVisual)(THIS_
        _In_ IDCompositionVisual* visual
        ) PURE;

    // Removes all visuals from the children list of another visual.
    STDMETHOD(RemoveAllVisuals)(THIS) PURE;

    // Sets the mode to use when composing the bitmap against the render target.
    STDMETHOD(SetCompositeMode)(THIS_
        _In_ enum DCOMPOSITION_COMPOSITE_MODE compositeMode
        ) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionSurface
//
//  Synopsis:
//      An IDCompositionSurface interface represents a wrapper around a DirectX
//      object, or a sub-rectangle of one of those objects.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionSurface
DECLARE_INTERFACE_IID_(IDCompositionSurface, IUnknown, "BB8A4953-2C99-4F5A-96F5-4819027FA3AC")
{
    STDMETHOD(QueryInterface)(THIS_
        REFIID riid,
        _Outptr_ void** ppObject
        ) PURE;

    STDMETHOD(AddRef)(THIS) PURE;
    STDMETHOD(Release)(THIS) PURE;

    STDMETHOD(BeginDraw)(THIS_
        _In_opt_ const RECT *updateRect,
        _In_ REFIID iid,
        _Outptr_ void **updateObject,
        _Out_ POINT *updateOffset
        ) PURE;

    STDMETHOD(EndDraw)(THIS
        ) PURE;

    STDMETHOD(SuspendDraw)(THIS
        ) PURE;

    STDMETHOD(ResumeDraw)(THIS
        ) PURE;

    STDMETHOD(Scroll)(THIS_
        _In_opt_ const RECT *scrollRect,
        _In_opt_ const RECT *clipRect,
        _In_ int offsetX,
        _In_ int offsetY
        ) PURE;
};

//+-----------------------------------------------------------------------------
//
//  Interface:
//      IDCompositionVirtualSurface
//
//  Synopsis:
//      An IDCompositionVirtualSurface interface represents a sparsely
//      allocated surface.
//
//------------------------------------------------------------------------------
#undef INTERFACE
#define INTERFACE IDCompositionVirtualSurface
DECLARE_INTERFACE_IID_(IDCompositionVirtualSurface, IDCompositionSurface, "AE471C51-5F53-4A24-8D3E-D0C39C30B3F0")
{
    // Base interface methods
    STDMETHOD(QueryInterface)(THIS_
        REFIID riid,
        _Outptr_ void** ppObject
        ) PURE;

    STDMETHOD(AddRef)(THIS) PURE;
    STDMETHOD(Release)(THIS) PURE;

    STDMETHOD(BeginDraw)(THIS_
        _In_opt_ const RECT *updateRect,
        _In_ REFIID iid,
        _Outptr_ void **updateObject,
        _Out_ POINT *updateOffset
        ) PURE;

    STDMETHOD(EndDraw)(THIS
        ) PURE;

    STDMETHOD(SuspendDraw)(THIS
        ) PURE;

    STDMETHOD(ResumeDraw)(THIS
        ) PURE;

    STDMETHOD(Scroll)(THIS_
        _In_opt_ const RECT *scrollRect,
        _In_opt_ const RECT *clipRect,
        _In_ int offsetX,
        _In_ int offsetY
        ) PURE;

    // Derived interface methods
    STDMETHOD(Resize)(THIS_
        _In_ UINT width,
        _In_ UINT height
        ) PURE;

    STDMETHOD(Trim)(THIS_
        _In_reads_opt_(count) const RECT *rectangles,
        _In_ UINT count
        ) PURE;
};

#undef INTERFACE
#endif // NTDDI_WIN8

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion
