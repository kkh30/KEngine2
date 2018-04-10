#include "KEWindow.h"




KEWindow::KEWindow(const KEWindowDesc& p_desc):m_desc(p_desc)
{
}

KEWindow::KEWindow(KEWindowDesc && p_desc):m_desc(p_desc)
{
}

KEWindow::~KEWindow()
{
}

void KEWindow::OnStartUp()
{
	WSIWindow::SetTitle(m_desc.title.c_str());
	WSIWindow::SetWinSize(static_cast<uint16_t>(m_desc.width), static_cast<uint16_t>(m_desc.height));
	WSIWindow::SetWinPos(static_cast<uint16_t>(m_desc.x), static_cast<uint16_t>(m_desc.y));
}


