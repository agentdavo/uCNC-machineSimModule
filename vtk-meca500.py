#!/usr/bin/env python

import vtk
import math
from vtk.util.colors import *


filenames = ["meca500_base.stl","link1.stl","link2.stl","link3.stl","link4.stl","link5.stl","link6.stl","spindle_assy.stl"]


renWin = vtk.vtkRenderWindow()
meca500_assy = list()
actor = list()


class MyInteractor(vtk.vtkInteractorStyleTrackballCamera):
    def __init__(self, parent=None):
        self.AddObserver("CharEvent", self.OnCharEvent)
        self.AddObserver("KeyPressEvent", self.OnKeyPressEvent)

    def OnCharEvent(self, obj, event):
        pass

    def OnKeyPressEvent(self, obj, event):
        return


def LoadSTL(filename):
    reader = vtk.vtkSTLReader()
    reader.SetFileName(filename)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(reader.GetOutputPort())
    actor = vtk.vtkLODActor()
    actor.SetMapper(mapper)
    return actor


def CreateCoordinates():
    axes = vtk.vtkAxesActor()
    axes.SetTotalLength(100, 100, 100)
    axes.SetShaftType(0)
    axes.SetCylinderRadius(0.02)
    axes.GetXAxisCaptionActor2D().SetWidth(0.03)
    axes.GetYAxisCaptionActor2D().SetWidth(0.03)
    axes.GetZAxisCaptionActor2D().SetWidth(0.03)
    return axes


def SliderCallback1(obj, event):
    sliderRepres = obj.GetRepresentation()
    pos = sliderRepres.GetValue()
    meca500_assy[1].SetOrientation(0, 0, pos)
    renWin.Render()


def SliderCallback2(obj, event):
    sliderRepres = obj.GetRepresentation()
    pos = sliderRepres.GetValue()
    meca500_assy[2].SetOrientation(0, pos, 0)
    renWin.Render()


def SliderCallback3(obj, event):
    sliderRepres = obj.GetRepresentation()
    pos = sliderRepres.GetValue()
    meca500_assy[3].SetOrientation(0, pos, 0)
    renWin.Render()


def SliderCallback4(obj, event):
    sliderRepres = obj.GetRepresentation()
    pos = sliderRepres.GetValue()
    meca500_assy[4].SetOrientation(0, 0, pos)
    renWin.Render()


def SliderCallback5(obj, event):
    sliderRepres = obj.GetRepresentation()
    pos = sliderRepres.GetValue()
    meca500_assy[5].SetOrientation(0, -pos, 0)
    renWin.Render()


def SliderCallback6(obj, event):
    sliderRepres = obj.GetRepresentation()
    pos = sliderRepres.GetValue()
    meca500_assy[6].SetOrientation(0, 0, -pos)
    renWin.Render()


def GenterSliderRep(iren, position):
    slider_widget = ConfigSlider(
    vtk.vtkSliderRepresentation2D(), position)
    slider_widget.SetInteractor(iren)
    slider_widget.EnabledOn()
    return slider_widget


def ConfigSlider(sliderRep, Yaxes):
    sliderRep.SetMinimumValue(-175)
    sliderRep.SetMaximumValue(175)
    sliderRep.SetValue(0.0)  # Specify the current value for the widget
    sliderRep.GetSliderProperty().SetColor(1, 0, 0)
    sliderRep.GetSelectedProperty().SetColor(0, 0, 1)
    sliderRep.GetTubeProperty().SetColor(1, 1, 0)  # Change the color of the bar
    sliderRep.GetCapProperty().SetColor(0, 1, 1)
    sliderRep.GetPoint1Coordinate().SetCoordinateSystemToDisplay()
    sliderRep.GetPoint1Coordinate().SetValue(50, Yaxes)
    sliderRep.GetPoint2Coordinate().SetCoordinateSystemToDisplay()
    sliderRep.GetPoint2Coordinate().SetValue(400, Yaxes)
    sliderRep.SetSliderLength(0.02)
    sliderRep.SetSliderWidth(0.02)
    sliderRep.SetTubeWidth(0.005)
    sliderRep.SetEndCapWidth(0.03)
    sliderRep.ShowSliderLabelOn()  # display the slider text label
    sliderRep.SetLabelFormat("%.1f")
    sliderWidget = vtk.vtkSliderWidget()
    sliderWidget.SetRepresentation(sliderRep)
    sliderWidget.SetAnimationModeToAnimate()
    return sliderWidget


def CreateGround():
    # create plane source
    plane = vtk.vtkPlaneSource()
    plane.SetXResolution(5)
    plane.SetYResolution(5)
    plane.SetCenter(0.30, 0, 0.02)
    plane.SetNormal(0, 0, 1)
    # mapper
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(plane.GetOutputPort())

    # actor
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.GetProperty().SetRepresentationToWireframe()

    actor.GetProperty().SetColor(light_grey)
    transform = vtk.vtkTransform()
    transform.Scale(500, 500, 1)
    actor.SetUserTransform(transform)
    return actor


def CreateScene():
    ren = vtk.vtkRenderer()
    renWin.AddRenderer(ren)
    iren = vtk.vtkRenderWindowInteractor()
    iren.SetRenderWindow(renWin)
    style = MyInteractor()
    style.SetDefaultRenderer(ren)
    iren.SetInteractorStyle(style)

    for id, file in enumerate(filenames):
        actor.append(LoadSTL(file))
        actor[id].GetProperty().SetDiffuseColor(0.9, 0.9, 0.9)
        actor[id].GetProperty().SetDiffuse(.8)
        actor[id].GetProperty().SetSpecular(.5)
        actor[id].GetProperty().SetSpecularColor(1.0, 1.0, 1.0)
        actor[id].GetProperty().SetSpecularPower(30.0)

        tmp_assembly = vtk.vtkAssembly()
        meca500_assy.append(tmp_assembly)
        meca500_assy[id].AddPart(actor[id])
        if(id > 0):
            meca500_assy[id-1].AddPart(tmp_assembly)

    meca500_assy[0].SetOrigin(0,0,0)
    meca500_assy[1].SetOrigin(0,0,135)
    meca500_assy[2].SetOrigin(0,0,135)
    meca500_assy[3].SetOrigin(135,0,135)
    meca500_assy[4].SetOrigin(173,0,50)
    meca500_assy[5].SetOrigin(173,0,15)
    meca500_assy[6].SetOrigin(173,0,-55)

    meca500_assy[0].SetPosition(0,0,0)
    meca500_assy[1].SetPosition(0,0,0)
    meca500_assy[2].SetPosition(0,0,0)
    meca500_assy[3].SetPosition(0,0,0)
    meca500_assy[4].SetPosition(0,0,0)
    meca500_assy[5].SetPosition(0,0,0)
    meca500_assy[6].SetPosition(0,0,0)

    ren.AddActor(meca500_assy[0])

    # Add coordinates
    axes = CreateCoordinates()
    ren.AddActor(axes)

    # Add ground
    ground = CreateGround()
    ren.AddActor(ground)

    slider_1 = GenterSliderRep(iren,40)
    slider_2 = GenterSliderRep(iren,80)
    slider_3 = GenterSliderRep(iren,120)
    slider_4 = GenterSliderRep(iren,160)
    slider_5 = GenterSliderRep(iren,200)
    slider_6 = GenterSliderRep(iren,240)

    slider_1.AddObserver("InteractionEvent", SliderCallback1)
    slider_2.AddObserver("InteractionEvent", SliderCallback2)
    slider_3.AddObserver("InteractionEvent", SliderCallback3)
    slider_4.AddObserver("InteractionEvent", SliderCallback4)
    slider_5.AddObserver("InteractionEvent", SliderCallback5)
    slider_6.AddObserver("InteractionEvent", SliderCallback6)

    # Set background color
    ren.SetBackground(.2, .2, .2)

    # Set window size
    renWin.SetSize(600, 600)

    # Set up the camera to get a particular view of the scene
    camera = vtk.vtkCamera()
    camera.SetFocalPoint(150, 0, 0)
    camera.SetPosition(400, 100, 400)
    camera.ComputeViewPlaneNormal()
    camera.SetViewUp(0, 0, 1)
    camera.Zoom(0.3)
    ren.SetActiveCamera(camera)
    iren.Initialize()
    iren.Start()


if __name__ == "__main__":
    CreateScene()
