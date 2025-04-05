import asyncio
import logging
import os
import threading

import Utils

if Utils.is_frozen():
    os.environ["KIVY_DATA_DIR"] = Utils.local_path("data")

from kvui import ContainerLayout, UILog, MainLayout, TooltipLabel
from kivy.app import App
from kivy.uix.layout import Layout
from kivy.uix.recycleview.views import RecycleDataViewBehavior

from worlds.AutoWorld import World
from worlds.LauncherComponents import Component, Type, components, launch_subprocess
from . import DumpOptions


class SingleLabel(RecycleDataViewBehavior, TooltipLabel):
    pass


class WizardConsoleGui(App):
    container: Layout
    exit_event: asyncio.locks.Event
    t1: threading.Thread

    def __init__(self):
        self.title = "Dump Options"
        self.exit_event = asyncio.Event()

        super().__init__()

    def build(self) -> Layout:
        self.container = ContainerLayout()

        ui_log = UILog(logging.getLogger())
        ui_log.viewclass = SingleLabel

        grid = MainLayout()
        grid.cols = 1
        grid.add_widget(ui_log)
        self.container.add_widget(grid)

        return self.container

    def on_start(self):
        t1 = threading.Thread(target=DumpOptions.dump, args=["dumped-options.json"])
        t1.start()

    def on_stop(self):
        self.exit_event.set()


def launch_client_helper():
    async def main():
        gui = WizardConsoleGui()
        gui_task = asyncio.create_task(gui.async_run(), name="DumpOptionsGui")

        await gui.exit_event.wait()
        await gui_task

    asyncio.run(main())


def launch_client():
    launch_subprocess(launch_client_helper, "DumpOptions")


components.append(Component("Dump Options for Wizard", "DumpOptions", func=launch_client, component_type=Type.TOOL))


class ApWizardWorld(World):
    item_name_to_id = {}
    location_name_to_id = {}
