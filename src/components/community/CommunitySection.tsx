import Link from "next/link";
import { ArrowRight } from "lucide-react";
import type { CommunityPerson } from "@/data/community";
import PersonCard from "./PersonCard";

export default function CommunitySection({
  title,
  action,
  href,
  people,
  disableHover = false,
}: {
  title: string;
  action: string;
  href: string;
  people: CommunityPerson[];
  disableHover?: boolean;
}) {
  return (
    <section className="border-t border-[#d95b00]/25 py-14">

      <div className="grid grid-cols-5 gap-6 max-xl:grid-cols-3 max-md:grid-cols-2 max-sm:grid-cols-1">
        {people.map((person) => (
          <PersonCard key={person.name} person={person} disableHover={disableHover} />
        ))}
      </div>
    </section>
  );
}