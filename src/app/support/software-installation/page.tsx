import type { Metadata } from "next";
import { Download } from "lucide-react";
import Header from "@/components/Header";
import Footer from "@/components/Footer";

export const metadata: Metadata = {
  title: "Software Installation",
};

export default function SoftwareInstallationPage() {
  return (
    <main className="min-h-screen bg-black text-white">
      <section className="bg-slate-950 px-6 py-20 text-white">
        <div className="mx-auto max-w-4xl">
          <p className="font-semibold text-orange-300">Support Article</p>
          <h1 className="mt-4 text-5xl font-bold">
            Advayam Software Installation Guide
          </h1>
          <p className="mt-6 text-lg text-slate-300">
            Install the Advayam VS Code Extension, configure board support, and
            access additional tools for embedded development.
          </p>
        </div>
      </section>

      <article className="mx-auto max-w-4xl px-6 py-16">
        <h2 className="text-3xl font-bold">Installation Steps</h2>

        <div className="mt-8 space-y-8">
          <Step
            title="1. Install Visual Studio Code"
            text="Download and install Visual Studio Code on your system."
          />
          <Step
            title="2. Install Advayam VS Code Extension"
            text="Open VS Code, go to Extensions, search for Advayam, and install the official extension."
          />
          <Step
            title="3. Connect Your Advayam Board"
            text="Connect the board using a data-supported USB cable and select the correct port."
          />
          <Step
            title="4. Create or Open a Project"
            text="Use Advayam: Create Project from the Command Palette to generate a ready-to-use embedded project."
          />
          <Step
            title="5. Build and Upload"
            text="Compile the project and upload it to the connected Advayam board."
          />
        </div>

        <section className="mt-16 rounded-3xl bg-slate-50 p-8">
          <h2 className="text-3xl font-bold">Additional Download Options</h2>

          <div className="mt-6 grid gap-4">
            <DownloadButton label="Download VS Code Extension" />
            <DownloadButton label="Download Web IDE Package" />
            <DownloadButton label="Download SDKs" />
            <DownloadButton label="Download Board Drivers" />
            <DownloadButton label="Download Documentation PDF" />
          </div>
        </section>
      </article>
    </main>
  );
}

function Step({ title, text }: { title: string; text: string }) {
  return (
    <div className="border-l-4 border-orange-500 pl-5">
      <h3 className="text-xl font-bold">{title}</h3>
      <p className="mt-2 text-slate-600">{text}</p>
    </div>
  );
}

function DownloadButton({ label }: { label: string }) {
  return (
    <a
      href="#"
      className="inline-flex items-center gap-2 rounded-full bg-slate-900 px-5 py-3 font-semibold text-white hover:bg-slate-700"
    >
      <Download className="h-5 w-5" />
      {label}
    </a>
  );
}
