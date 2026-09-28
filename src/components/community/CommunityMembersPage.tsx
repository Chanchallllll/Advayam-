"use client";

import { useMemo, useState } from "react";
import CommunityHeader from "./CommunityHeader";
import PersonCard from "./PersonCard";
import type { CommunityPerson } from "@/data/community";

export default function CommunityMembersPage({
  title,
  subtitle,
  people,
  disableHover = false,
}: {
  title: string;
  subtitle: string;
  people: CommunityPerson[];
  disableHover?: boolean;
}) {
  const [query, setQuery] = useState("");

  const filteredPeople = useMemo(() => {
    const value = query.trim().toLowerCase();
    if (!value) return people;

    return people.filter(
      (person) =>
        person.name.toLowerCase().includes(value) ||
        person.role.toLowerCase().includes(value)
    );
  }, [people, query]);

  return (
    <main className="relative min-h-screen overflow-hidden bg-black text-white">
      <CommunityHeader query={query} setQuery={setQuery} />

      <img
        src="/patterns/wave-right.svg"
        alt=""
        className="pointer-events-none fixed right-0 top-[92px] z-0 h-[430px] w-[520px] object-contain object-right opacity-25"
      />

      <section className="relative z-10 mx-auto max-w-[1500px] px-10 pb-16 pt-[145px] max-sm:px-5">
        <h1 className="font-jakarta text-6xl font-black leading-none text-white max-md:text-4xl">
          {title}
        </h1>

        <p className="mt-5 font-jakarta text-lg font-medium text-white/70">
          {subtitle}
        </p>

        {query && (
          <p className="mt-6 font-jakarta text-sm font-bold text-orange-400">
            Showing {filteredPeople.length} result(s) for “{query}”
          </p>
        )}

        <div className="mt-14 grid grid-cols-5 gap-7 max-2xl:grid-cols-4 max-xl:grid-cols-3 max-md:grid-cols-2 max-sm:grid-cols-1">
          {filteredPeople.map((person) => (
            <PersonCard key={person.name} person={person} disableHover={disableHover} />
          ))}
        </div>

        {filteredPeople.length === 0 && (
          <div className="mt-20 rounded-2xl border border-white/10 bg-white/[0.05] p-10 text-center shadow-sm">
            <h2 className="font-jakarta text-2xl font-black">
              No members found
            </h2>
            <p className="mt-3 font-jakarta text-sm font-medium text-white/60">
              Try searching another name or role.
            </p>
          </div>
        )}
      </section>
    </main>
  );
}