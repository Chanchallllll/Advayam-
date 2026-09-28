import CommunityHeader from "@/components/community/CommunityHeader";

const steps = [
  "Open the Advayam GitHub link.",
  "Fork the Advayam repository.",
  "Make a small change in the README.md document of the forked repository.",
  "Commit your changes.",
  "Send a pull request from your forked repository to the main Advayam repository.",
  "The Advayam Lead will review and analyse your pull request.",
  "Once your pull request is accepted, you become our latest contributor!",
];

export default function ContributionGuidelinesPage() {
  return (
    <main className="min-h-screen bg-black text-white">
      <CommunityHeader />

      <section className="mx-auto max-w-[1100px] px-8 pb-20 pt-[145px]">
        <p className="font-jakarta text-sm font-black uppercase tracking-[0.18em] text-orange-400">
          Community Workflow
        </p>

        <h1 className="mt-4 font-jakarta text-5xl font-black">
          Contribution Guidelines
        </h1>

        <p className="mt-5 max-w-3xl font-jakarta text-base font-medium leading-7 text-white/70">
          Follow this workflow to become a contributor in the Advayam GitHub
          community.
        </p>

        <a
          href="https://github.com/Webyfy/Advayam-IDE"
          target="_blank"
          rel="noreferrer"
          className="mt-8 inline-flex rounded-xl bg-[#BE5209] px-6 py-3 font-jakarta text-sm font-black text-white"
        >
          Open GitHub Link
        </a>

        <div className="mt-12 grid gap-5">
          {steps.map((step, index) => (
            <div
              key={step}
              className="flex gap-5 rounded-3xl border border-white/10 bg-white/[0.05] p-6 shadow-sm"
            >
              <span className="flex h-10 w-10 shrink-0 items-center justify-center rounded-full bg-[#d95b00] font-jakarta text-sm font-black text-white">
                {index + 1}
              </span>

              <p className="font-jakarta text-base font-medium leading-7 text-white">
                {step}
              </p>
            </div>
          ))}
        </div>
      </section>
    </main>
  );
}