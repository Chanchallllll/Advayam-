"use client";

import { useMemo, useState } from "react";
import CommunityHeader from "@/components/community/CommunityHeader";
import CommunitySection from "@/components/community/CommunitySection";
import { contributors, mentors } from "@/data/community";

export default function CommunityPage() {
  const [query, setQuery] = useState("");

  function filterPeople(people: typeof mentors) {
    const value = query.trim().toLowerCase();
    if (!value) return people;

    return people.filter(
      (person) =>
        person.name.toLowerCase().includes(value) ||
        person.role.toLowerCase().includes(value)
    );
  }

  const filteredMentors = useMemo(() => filterPeople(mentors), [query]);
  const filteredContributors = useMemo(
    () => filterPeople(contributors),
    [query]
  );

  return (
    <main className="relative min-h-screen overflow-hidden bg-black text-white">
      <CommunityHeader query={query} setQuery={setQuery} />

      <img
        src="/patterns/wave-right-yellow.svg"
        alt=""
        className="pointer-events-none fixed right-0 top-[92px] z-0 h-[430px] w-[520px] object-contain object-right opacity-125"
      />

      <section className="relative z-10 mx-auto max-w-[1500px] px-10 pb-16 pt-[145px] max-sm:px-5">
        <h1 className="font-jakarta text-6xl font-black leading-none max-md:text-4xl">
          Our Community
        </h1>

        <p className="mt-5 font-jakarta text-lg font-medium text-white/70">
          Connect, collaborate and grow with the Advayam community.
        </p>

        <div className="mt-16">
          <CommunitySection
            title="Mentors"
            action="View All Mentors"
            href="/community/mentors"
            people={filteredMentors}
          />

          <CommunitySection
            title="Contributors"
            action="View All Contributors"
            href="/community/contributors"
            people={filteredContributors}
            disableHover
          />
        </div>
      </section>
    </main>
  );
}