import CommunityMembersPage from "@/components/community/CommunityMembersPage";
import { contributors } from "@/data/community";

export default function ContributorsPage() {
  return (
    <CommunityMembersPage
      title="All Contributors"
      subtitle="Meet the developers, designers, writers, and innovators building Advayam together."
      people={contributors}
      disableHover
    />
  );
}