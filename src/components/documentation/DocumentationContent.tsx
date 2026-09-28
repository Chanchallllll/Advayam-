import { documentationSections } from "@/data/documentation";
import DocumentationSection from "./DocumentationSection";

export default function DocumentationContent() {
  return (
    <article className="mx-auto max-w-4xl bg-[#0c0c0c] pb-32 pt-8 sm:pt-10">

      <h1 className="font-jakarta text-4xl font-black text-white sm:text-5xl md:text-6xl">
        Hardware
      </h1>

      <p className="font-jakarta mt-5 max-w-4xl text-sm font-medium leading-7 text-white/65 sm:mt-6 sm:text-base">
        Explore the building blocks of modern embedded systems. Learn about
        microcontrollers, development boards, sensors, communication modules,
        power systems, and electronic components through interactive guides and
        practical examples.
      </p>

      <h2 className="font-jakarta mt-12 text-2xl font-black text-white sm:mt-16 sm:text-3xl">
        Featured Hardware Cards
      </h2>

      <div className="mt-8 space-y-14 sm:mt-10 sm:space-y-16">
        {documentationSections.map((section) => (
          <DocumentationSection
            key={section.id}
            {...section}
          />
        ))}
      </div>
    </article>
  );
}