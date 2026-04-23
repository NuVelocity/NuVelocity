target_sources(
    NuVelocity PRIVATE
    WidgetUtils.cpp
    WidgetUtils.h
    Widget.cpp
    Widget.h
    Button.cpp
    Button.h
    FocusContainer.cpp
    FocusContainer.h
    TextBox.cpp
    TextBox.h
    MdiWindow.cpp
    MdiWindow.h
    MdiManager.cpp
    MdiManager.h
    ButtonContainer.cpp
    ButtonContainer.h
)

add_subdirectory(skin)
