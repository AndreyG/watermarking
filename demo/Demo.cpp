// Demo.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include "graph.h"

using cg::colorf;

struct Viewer
   : viewer::ViewerImpl
{
   Viewer()
      : ViewerImpl( "demo" )
   {
	   colors_.push_back(colorf(1, 0, 0));
	   colors_.push_back(colorf(0, 1, 0));
	   colors_.push_back(colorf(0, 0, 1));
	   colors_.push_back(colorf(0.7, 0.7, 0));
	   colors_.push_back(colorf(0.7, 0.0, 0.7));
   }

   void Init(std::vector< graph_t > const * graphs)
   {
	   graphs_ = graphs;
	   Assert(colors_.size() >= graphs_->size());
	   visible_.resize(graphs->size(), false);
   }

   void __stdcall OnDraw()
   {
      point_2 mouse_pos = dc_.getMousePos();
		
	  for (size_t k = 0; k != graphs_->size(); ++k)
	  {
		  if (!visible_[k])
			  continue;

		  graph_t const & graph = graphs_->at(k);
		  dc_.setColor(colors_[k]);

		  for (size_t i = 0, N = vertices_num(graph); i != N; ++i)
			  dc_.drawPixel(graph.vertices[i], 2);
		  for (size_t i = 0, N = edges_num(graph); i != N; ++i)
			  dc_.drawLine(graph.vertices[graph.edges[i].first], graph.vertices[graph.edges[i].second]);
	  }

      viewer::printer prn(dc_, point_2i(10, 15));
      prn << mouse_pos;
	  for (size_t k = 0; k != visible_.size(); ++k)
		  if (visible_[k])
			  prn << "draw " << k << "-th graph";
   }

   // keyboard
   BOOL __stdcall OnKey(DWORD key, BOOL down, BOOL * redraw)
   {
	   if (!down)
	   {
		   if (key >= '1' && key <= '9')
		   {
			   int idx = key - '1';
			   if (idx < (int)visible_.size())
			   {
				   visible_[idx] = !visible_[idx];
				   *redraw = TRUE;
				   return TRUE;
			   }
		   }
	   }
	   return FALSE;
   }

   void __stdcall OnMove( BOOL * redraw ) 
   {
      *redraw = TRUE;
   }

   // mouse button
   BOOL __stdcall OnButton( BOOL left, BOOL down, BOOL * redraw )
   {
      return FALSE;
   } 

private:
	std::vector< graph_t > const * graphs_;
	std::vector< cg::colorf > colors_;
	std::vector< bool > visible_;
};

int main(int argc, char* argv[])
{
   ::CoInitialize(NULL);

   std::vector< graph_t > graphs;
   for (int i = 1; i != argc; ++i)
   {
	   std::ifstream in(argv[i]);
	   graphs.push_back(graph_t(in));
   }
   fix_graph(graphs[0]);

   CComObject<Viewer> * ptr = 0;
   CComObject<Viewer>::CreateInstance(&ptr);
   
   ptr->AddRef();
   ptr->Init(&graphs);
   ptr->DoModal();
   ptr->Release();

   ::CoUninitialize();
   return 0;
}



