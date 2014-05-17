/**
 * @file UMDirectX11GUIBoard.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <dxgi.h>
#include <d3d11.h>

#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMMesh.h"

namespace umdraw
{
	class UMDirectX11Mesh;
	typedef std::shared_ptr<UMDirectX11Mesh> UMDirectX11MeshPtr;

	class UMDirectX11DrawParameter;
	typedef std::shared_ptr<UMDirectX11DrawParameter> UMDirectX11DrawParameterPtr;
} // umdraw

namespace umgui
{
	
class UMGUIBoard;
typedef std::shared_ptr<UMGUIBoard> UMGUIBoardPtr;
typedef std::weak_ptr<UMGUIBoard> UMGUIBoardWeakPtr;

class UMDirectX11GUIBoard;
typedef std::shared_ptr<UMDirectX11GUIBoard> UMDirectX11GUIBoardPtr;
typedef std::vector<UMDirectX11GUIBoardPtr> UMDirectX11GUIBoardList;

/**
 * a node
 */
class UMDirectX11GUIBoard
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11GUIBoard);
public:
	UMDirectX11GUIBoard() {}
	UMDirectX11GUIBoard(UMGUIBoardPtr board);

	~UMDirectX11GUIBoard() {}

	/**
	 * initialize
	 */
	bool init(ID3D11Device* device_pointer);
	
	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer, umdraw::UMDirectX11DrawParameterPtr parameter) const;

private:
	UMGUIBoardWeakPtr board_;
	umdraw::UMDirectX11MeshPtr dx_mesh_;
};

} // umgui
